#ifdef _WIN32
#include <windows.h>
#endif

#include <QApplication>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Message.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GraphicDriver.hxx>

#include "OcctFrameBuffer.h"
#include "OcctGlTools.h"
#include "OcctQtViewer.h"

OcctQtViewer::OcctQtViewer(bool theIsCoreProfile, QWidget *theParent)
    : QOpenGLWidget(theParent)
    , myIsCoreProfile(theIsCoreProfile)
{
    Handle(Aspect_DisplayConnection) aDisp = new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver) aDriver = new OpenGl_GraphicDriver(aDisp, false);
    // lets QOpenGLWidget to manage buffer swap
    aDriver->ChangeOptions().buffersNoSwap = true;
    // don't write into alpha channel
    aDriver->ChangeOptions().buffersOpaqueAlpha = true;
    // offscreen FBOs should be always used
    aDriver->ChangeOptions().useSystemBuffer = false;

    // create viewer
    myViewer = new V3d_Viewer(aDriver);
    myViewer->SetDefaultBackgroundColor(Quantity_NOC_BLACK);
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();
    myViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

    // create AIS context
    myContext = new AIS_InteractiveContext(myViewer);

    myViewCube = new AIS_ViewCube();
    myViewCube->SetViewAnimation(myViewAnimation);
    myViewCube->SetFixedAnimationLoop(false);
    myViewCube->SetAutoStartAnimation(true);
    myViewCube->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(100, 150));

    // note - window will be created later within initializeGL() callback!
    myView = myViewer->CreateView();
    myView->SetImmediateUpdate(false);
#ifndef __APPLE__
    myView->ChangeRenderingParams().NbMsaaSamples = 4; // warning - affects performance
#endif
    myView->ChangeRenderingParams().ToShowStats = true;
    myView->ChangeRenderingParams().CollectedStats =
        (Graphic3d_RenderingParams::
             PerfCounters)(Graphic3d_RenderingParams::PerfCounters_FrameRate |
                           Graphic3d_RenderingParams::PerfCounters_Triangles);

    // Qt widget setup
    setMouseTracking(true);
    setBackgroundRole(QPalette::NoRole); // or NoBackground
    setFocusPolicy(Qt::StrongFocus); // set focus policy to threat QContextMenuEvent from keyboard
    setUpdatesEnabled(true);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    // OpenGL setup managed by Qt
    QSurfaceFormat aGlFormat;
    aGlFormat.setDepthBufferSize(24);
    aGlFormat.setStencilBufferSize(8);
    // aGlFormat.setOption (QSurfaceFormat::DebugContext, true);
    aDriver->ChangeOptions().contextDebug = aGlFormat.testOption(QSurfaceFormat::DebugContext);
    // aGlFormat.setOption (QSurfaceFormat::DeprecatedFunctions, true);

    // Use compatibility profile and fallback version for WSL2
    if (myIsCoreProfile)
    {
        aGlFormat.setVersion(4, 5);
        aGlFormat.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(aGlFormat);
    }
    else
    {
        // Use a more compatible version for WSL2/D3D12
        aGlFormat.setVersion(3, 3);
        aGlFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
        QSurfaceFormat::setDefaultFormat(aGlFormat);
    }

    setFormat(aGlFormat);
}

OcctQtViewer::~OcctQtViewer()
{
    // hold on X11 display connection till making another connection active by glXMakeCurrent()
    // to workaround sudden crash in QOpenGLWidget destructor
    Handle(Aspect_DisplayConnection) aDisp = myViewer->Driver()->GetDisplayConnection();

    // release OCCT viewer
    myContext->RemoveAll(false);
    myContext.Nullify();
    myView->Remove();
    myView.Nullify();
    myViewer.Nullify();

    // make active OpenGL context created by Qt
    makeCurrent();
    aDisp.Nullify();
}

void OcctQtViewer::initializeGL()
{
    const QRect aRect = rect();
    const Graphic3d_Vec2i aViewSize(aRect.right() - aRect.left(), aRect.bottom() - aRect.top());

    Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
#ifdef _WIN32
    HDC aWglDevCtx = wglGetCurrentDC();
    HWND aWglWin = WindowFromDC(aWglDevCtx);
    aNativeWin = (Aspect_Drawable)aWglWin;
#endif

    Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();

    // Try first with the current profile setting
    bool contextInitialized = aGlCtx->Init(myIsCoreProfile);

    // If failed and we were trying core profile, fallback to compatibility profile
    if (!contextInitialized && myIsCoreProfile)
    {
        Message::SendWarning()
            << "Warning: Core profile context initialization failed, trying compatibility profile";
        myIsCoreProfile = false;
        contextInitialized = aGlCtx->Init(myIsCoreProfile);
    }

    if (!contextInitialized)
    {
        Message::SendFail()
            << "Error: OpenGl_Context is unable to wrap OpenGL context (tried both core and compatibility profiles)";
        QMessageBox::critical(nullptr, "OpenGL Context Failure",
                              "Unable to initialize OpenGL context.\n\n"
                              "This may be due to WSL2/D3D12 compatibility issues.\n"
                              "Try running on native Linux or with different graphics drivers.");
        QApplication::exit(1);
        return;
    }

    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
    if (!aWindow.IsNull())
    {
        aWindow->SetNativeHandle(aNativeWin);
        aWindow->SetSize(aViewSize.x(), aViewSize.y());
        myView->SetWindow(aWindow, aGlCtx->RenderingContext());
        dumpGlInfo(true, true);
    }
    else
    {
        aWindow = new Aspect_NeutralWindow();
        aWindow->SetVirtual(true);
        aWindow->SetNativeHandle(aNativeWin);
        aWindow->SetSize(aViewSize.x(), aViewSize.y());
        myView->SetWindow(aWindow, aGlCtx->RenderingContext());
        dumpGlInfo(true, true);

        myContext->Display(myViewCube, 0, 0, false);
    }

    {
        // dummy shape for testing
        TopoDS_Shape aBox = BRepPrimAPI_MakeBox(100.0, 50.0, 90.0).Shape();
        Handle(AIS_Shape) aShape = new AIS_Shape(aBox);
        myContext->Display(aShape, AIS_Shaded, 0, false);
    }
}

void OcctQtViewer::paintGL()
{
    if (myView->Window().IsNull())
    {
        return;
    }

    Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
#ifdef _WIN32
    HDC aWglDevCtx = wglGetCurrentDC();
    HWND aWglWin = WindowFromDC(aWglDevCtx);
    aNativeWin = (Aspect_Drawable)aWglWin;
#endif
    if (myView->Window()->NativeHandle() != aNativeWin)
    {
        // workaround window recreation done by Qt on monitor (QScreen) disconnection
        Message::SendWarning() << "Native window handle has changed by QOpenGLWidget!";
        initializeGL();
        return;
    }

    // wrap FBO created by QOpenGLWidget
    // get context from this (composer) view rather than from arbitrary one
    // Handle(OpenGl_GraphicDriver) aDriver = Handle(OpenGl_GraphicDriver)::DownCast
    // (myContext->CurrentViewer()->Driver()); Handle(OpenGl_Context) aGlCtx =
    // aDriver->GetSharedContext();
    Handle(OpenGl_Context) aGlCtx = OcctGlTools::GetGlContext(myView);
    Handle(OpenGl_FrameBuffer) aDefaultFbo = aGlCtx->DefaultFrameBuffer();
    if (aDefaultFbo.IsNull())
    {
        aDefaultFbo = new OcctQtFrameBuffer();
        aGlCtx->SetDefaultFrameBuffer(aDefaultFbo);
    }
    if (!aDefaultFbo->InitWrapper(aGlCtx))
    {
        aDefaultFbo.Nullify();
        Message::DefaultMessenger()->Send("Default FBO wrapper creation failed", Message_Fail);
        QMessageBox::critical(nullptr, "Failure", "Default FBO wrapper creation failed");
        QApplication::exit(1);
        return;
    }

    Graphic3d_Vec2i aViewSizeOld;
    // const QRect aRect = rect(); Graphic3d_Vec2i aViewSizeNew(aRect.right() - aRect.left(),
    // aRect.bottom() - aRect.top());
    Graphic3d_Vec2i aViewSizeNew = aDefaultFbo->GetVPSize();
    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
    aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
    if (aViewSizeNew != aViewSizeOld)
    {
        aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
        myView->MustBeResized();
        myView->Invalidate();
        dumpGlInfo(true, false);

        for (Handle(V3d_View) aSubviewIter : myView->Subviews())
        {
            aSubviewIter->MustBeResized();
            aSubviewIter->Invalidate();
            aDefaultFbo->SetupViewport(aGlCtx);
        }
    }

    // flush pending input events and redraw the viewer
    Handle(V3d_View) aView = !myFocusView.IsNull() ? myFocusView : myView;
    aView->InvalidateImmediate();
    FlushViewEvents(myContext, aView, true);
}

void OcctQtViewer::dumpGlInfo(bool theIsBasic, bool theToPrint)
{
    TColStd_IndexedDataMapOfStringString aGlCapsDict;
    myView->DiagnosticInformation(aGlCapsDict, theIsBasic ? Graphic3d_DiagnosticInfo_Basic :
                                                            Graphic3d_DiagnosticInfo_Complete);
    TCollection_AsciiString anInfo;
    for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aGlCapsDict); aValueIter.More();
         aValueIter.Next())
    {
        if (!aValueIter.Value().IsEmpty())
        {
            if (!anInfo.IsEmpty())
            {
                anInfo += "\n";
            }
            anInfo += aValueIter.Key() + ": " + aValueIter.Value();
        }
    }

    if (theToPrint)
    {
        Message::SendInfo(anInfo);
    }
    myGlInfo = QString::fromUtf8(anInfo.ToCString());
}

void OcctQtViewer::closeEvent(QCloseEvent *theEvent)
{
    theEvent->accept();
}

void OcctQtViewer::keyPressEvent(QKeyEvent *theEvent)
{
    Aspect_VKey aKey = OcctGlTools::qtKey2VKey(theEvent->key());
    switch (aKey)
    {
    case Aspect_VKey_Escape:
    {
        QApplication::exit();
        return;
    }
    case Aspect_VKey_F:
    {
        myView->FitAll(0.01, false);
        update();
        return;
    }
    }
    QOpenGLWidget::keyPressEvent(theEvent);
}

void OcctQtViewer::mousePressEvent(QMouseEvent *theEvent)
{
    QOpenGLWidget::mousePressEvent(theEvent);
    double scale = this->devicePixelRatio();
    const Graphic3d_Vec2i aPnt(
        Graphic3d_Vec2d(theEvent->pos().x() * scale, theEvent->pos().y() * scale));
    const Aspect_VKeyFlags aFlags = OcctGlTools::qtMouseModifiers2VKeys(theEvent->modifiers());
    if (!myView.IsNull() &&
        UpdateMouseButtons(aPnt, OcctGlTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags,
                           false))
    {
        updateView();
    }
}

void OcctQtViewer::mouseReleaseEvent(QMouseEvent *theEvent)
{
    QOpenGLWidget::mouseReleaseEvent(theEvent);
    double scale = this->devicePixelRatio();
    const Graphic3d_Vec2i aPnt(
        Graphic3d_Vec2d(theEvent->pos().x() * scale, theEvent->pos().y() * scale));
    const Aspect_VKeyFlags aFlags = OcctGlTools::qtMouseModifiers2VKeys(theEvent->modifiers());
    if (!myView.IsNull() &&
        UpdateMouseButtons(aPnt, OcctGlTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags,
                           false))
    {
        updateView();
    }
}

void OcctQtViewer::mouseMoveEvent(QMouseEvent *theEvent)
{
    QOpenGLWidget::mouseMoveEvent(theEvent);
    double scale = this->devicePixelRatio();
    const Graphic3d_Vec2i aNewPos(
        Graphic3d_Vec2d(theEvent->pos().x() * scale, theEvent->pos().y() * scale));
    if (!myView.IsNull() &&
        UpdateMousePosition(aNewPos, OcctGlTools::qtMouseButtons2VKeys(theEvent->buttons()),
                            OcctGlTools::qtMouseModifiers2VKeys(theEvent->modifiers()), false))
    {
        updateView();
    }
}

void OcctQtViewer::wheelEvent(QWheelEvent *theEvent)
{
    QOpenGLWidget::wheelEvent(theEvent);
    double scale = this->devicePixelRatio();
    const Graphic3d_Vec2i aPos(
        Graphic3d_Vec2d(theEvent->position().x() * scale, theEvent->position().y() * scale));

    if (myView.IsNull())
    {
        return;
    }

    if (!myView->Subviews().IsEmpty())
    {
        Handle(V3d_View) aPickedView = myView->PickSubview(aPos);
        if (!aPickedView.IsNull() && aPickedView != myFocusView)
        {
            // switch input focus to another subview
            OnSubviewChanged(myContext, myFocusView, aPickedView);
            updateView();
            return;
        }
    }

    if (UpdateZoom(Aspect_ScrollDelta(aPos, double(theEvent->angleDelta().y()) / 8.0)))
    {
        updateView();
    }
}

void OcctQtViewer::updateView()
{
    update();
}

void OcctQtViewer::handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                                    const Handle(V3d_View) & theView)
{
    AIS_ViewController::handleViewRedraw(theCtx, theView);
    if (myToAskNextFrame)
    {
        // ask more frames for animation
        updateView();
    }
}

void OcctQtViewer::OnSubviewChanged(const Handle(AIS_InteractiveContext) &,
                                    const Handle(V3d_View) &, const Handle(V3d_View) & theNewView)
{
    myFocusView = theNewView;
}
