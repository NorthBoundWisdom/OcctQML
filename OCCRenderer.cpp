#ifdef _WIN32
#include <windows.h>
#endif

#include <QGuiApplication>
#include <QMouseEvent>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Message.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GraphicDriver.hxx>

#include "OCCRenderer.h"
#include "OccQuickItem.h"
#include "OcctFrameBuffer.h"
#include "OcctGlTools.h"

namespace geotoys
{
OCCRenderer::OCCRenderer(OccQuickItem * /*item*/)
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

    QSurfaceFormat aGlFormat;
    aGlFormat.setDepthBufferSize(24);
    aGlFormat.setStencilBufferSize(8);
    aDriver->ChangeOptions().contextDebug = aGlFormat.testOption(QSurfaceFormat::DebugContext);

// Use compatibility profile and fallback version for WSL2
#if defined(_WIN32)
    myIsCoreProfile = true;
#else
    myIsCoreProfile = false;
#endif

    if (myIsCoreProfile)
    {
        aGlFormat.setVersion(4, 5);
        aGlFormat.setProfile(QSurfaceFormat::CoreProfile);
    }
    else
    {
        // Use a more compatible version for WSL2/D3D12
        aGlFormat.setVersion(3, 3);
        aGlFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
    }
}

OCCRenderer::~OCCRenderer()
{
    // hold on X11 display connection till making another connection active by
    // glXMakeCurrent() to workaround sudden crash in QOpenGLWidget destructor
    Handle(Aspect_DisplayConnection) aDisp = myViewer->Driver()->GetDisplayConnection();

    // release OCCT viewer
    myContext->RemoveAll(false);
    myContext.Nullify();
    myView->Remove();
    myView.Nullify();
    myViewer.Nullify();

    aDisp.Nullify();
}

void OCCRenderer::render()
{
    if (myView->Window().IsNull())
    {
        return;
    }

    // wrap FBO created by QQuickFramebufferObject
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
        return;
    }

    Graphic3d_Vec2i aViewSizeOld;
    Graphic3d_Vec2i aViewSizeNew = aDefaultFbo->GetVPSize();
    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
    aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
    if (aViewSizeNew != aViewSizeOld)
    {
        aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
        myView->MustBeResized();
        myView->Invalidate();

        for (Handle(V3d_View) aSubviewIter : myView->Subviews())
        {
            aSubviewIter->MustBeResized();
            aSubviewIter->Invalidate();
            aDefaultFbo->SetupViewport(aGlCtx);
        }
    }

    myView->InvalidateImmediate();
    FlushViewEvents(myContext, myView, true);
}

QOpenGLFramebufferObject *OCCRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);

    // Initialize OpenGL context if needed
    if (myView->Window().IsNull())
    {
        initializeGL(size);
    }

    return new QOpenGLFramebufferObject(size, format);
}

void OCCRenderer::initializeGL(const QSize &size)
{
    const Graphic3d_Vec2i aViewSize(size.width(), size.height());

    Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();

    // Try first with the current profile setting
    bool contextInitialized = aGlCtx->Init(myIsCoreProfile);

    // If failed and we were trying core profile, fallback to compatibility
    // profile
    if (!contextInitialized && myIsCoreProfile)
    {
        Message::SendWarning() << "Warning: Core profile context initialization "
                                  "failed, trying compatibility profile";
        myIsCoreProfile = false;
        contextInitialized = aGlCtx->Init(myIsCoreProfile);
    }

    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
    if (!aWindow.IsNull())
    {
        aWindow->SetSize(aViewSize.x(), aViewSize.y());
        myView->SetWindow(aWindow, aGlCtx->RenderingContext());
    }
    else
    {
        aWindow = new Aspect_NeutralWindow();
        aWindow->SetVirtual(true);
        aWindow->SetSize(aViewSize.x(), aViewSize.y());
        myView->SetWindow(aWindow, aGlCtx->RenderingContext());
    }
}
} // namespace geotoys
