#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QScreen>
#include <QTime>

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
#include "OccViewerItem.h"
#include "OcctFrameBuffer.h"
#include "OcctGlTools.h"

namespace geotoys
{
OCCRenderer::OCCRenderer(const OccViewerItem *item)
    : quick_item_(item)
    , scene_manager_(new OccSceneManager())
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
    viewer_ = new V3d_Viewer(aDriver);
    viewer_->SetDefaultBackgroundColor(Quantity_NOC_BLACK);
    viewer_->SetDefaultLights();
    viewer_->SetLightOn();
    viewer_->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);

    view_cube_ = new AIS_ViewCube();
    view_cube_->SetViewAnimation(myViewAnimation);
    view_cube_->SetFixedAnimationLoop(false);
    view_cube_->SetAutoStartAnimation(true);
    view_cube_->TransformPersistence()->SetOffset2d(Graphic3d_Vec2i(100, 150));

    // create AIS context
    context_ = new AIS_InteractiveContext(viewer_);

    // note - window will be created later within initializeGL() callback!
    view_ = viewer_->CreateView();
    view_->SetImmediateUpdate(false);
    view_->ChangeRenderingParams().ToShowStats = true;
    view_->ChangeRenderingParams().CollectedStats =
        (Graphic3d_RenderingParams::
             PerfCounters)(Graphic3d_RenderingParams::PerfCounters_FrameRate |
                           Graphic3d_RenderingParams::PerfCounters_Triangles);

    view_->ChangeRenderingParams().NbMsaaSamples = 0;
    view_->ChangeRenderingParams().RenderResolutionScale = 1.0;
    view_->ChangeRenderingParams().ToEnableDepthPrepass = false;
    view_->ChangeRenderingParams().ToEnableAlphaToCoverage = false;

    scene_manager_->setContext(context_);
    scene_manager_->setView(view_);
}

OCCRenderer::~OCCRenderer()
{
    Handle(Aspect_DisplayConnection) aDisp = viewer_->Driver()->GetDisplayConnection();

    // release OCCT viewer
    context_->RemoveAll(false);
    context_.Nullify();
    view_->Remove();
    view_.Nullify();
    viewer_.Nullify();
    aDisp.Nullify();

    if (scene_manager_)
    {
        delete scene_manager_;
        scene_manager_ = nullptr;
    }
}

void OCCRenderer::synchronize(QQuickFramebufferObject *item)
{
    scale_ = item->window()->devicePixelRatio();
}

void OCCRenderer::render()
{
    QString times = "[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "]";
    std::cout << times.toStdString() << " render" << std::endl;

    if (view_->Window().IsNull())
    {
        return;
    }

    // wrap FBO created by QQuickFramebufferObject
    Handle(OpenGl_Context) aGlCtx = OcctGlTools::GetGlContext(view_);
    Handle(OpenGl_FrameBuffer) aDefaultFbo = aGlCtx->DefaultFrameBuffer();
    if (aDefaultFbo.IsNull())
    {
        std::cout << "[OCCRenderer] Creating new framebuffer" << std::endl;
        aDefaultFbo = new OcctQtFrameBuffer();
        aGlCtx->SetDefaultFrameBuffer(aDefaultFbo);
    }
    if (!aDefaultFbo->InitWrapper(aGlCtx))
    {
        std::cout << "[render] aDefaultFbo->InitWrapper(aGlCtx) failed" << std::endl;
        aDefaultFbo.Nullify();
        return;
    }

    aDefaultFbo->SetupViewport(aGlCtx);

    Graphic3d_Vec2i aViewSizeOld;
    Graphic3d_Vec2i aViewSizeNew = aDefaultFbo->GetVPSize();
    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(view_->Window());
    aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
    if (aViewSizeNew != aViewSizeOld)
    {
        aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
        view_->MustBeResized();
        view_->Invalidate();

        for (Handle(V3d_View) aSubviewIter : view_->Subviews())
        {
            aSubviewIter->MustBeResized();
            aSubviewIter->Invalidate();
            aDefaultFbo->SetupViewport(aGlCtx);
        }
    }
    if (pending_fit_all_)
    {
        view_->FitAll();
        view_->ZFitAll();
        pending_fit_all_ = false;
    }

    // Only display viewcube once during initialization, not every frame
    // context_->Display(view_cube_, 0, 0, false);
    view_->InvalidateImmediate();
    FlushViewEvents(context_, view_, true);
}

QOpenGLFramebufferObject *OCCRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(0);
    format.setTextureTarget(GL_TEXTURE_2D);
    format.setInternalTextureFormat(GL_RGBA8);

    if (view_->Window().IsNull())
    {
        std::cout << "[OCCRenderer] Initializing OpenGL context\n";
        initializeGL(size);
    }

    return new QOpenGLFramebufferObject(size, format);
}

void OCCRenderer::initializeGL(const QSize &fboSize)
{
    const Graphic3d_Vec2i aViewSize(fboSize.width(), fboSize.height());

    // Get native window handle
    Aspect_Drawable aNativeWin;
#ifdef _WIN32
    std::cout << "[initializeGL] wglGetCurrentDC\n";
    is_core_profile_ = true;
    HDC aWglDevCtx = wglGetCurrentDC();
    HWND aWglWin = WindowFromDC(aWglDevCtx);
    aNativeWin = (Aspect_Drawable)aWglWin;
#else
    is_core_profile_ = false;
    if (quick_item_ && quick_item_->window())
    {
        aNativeWin = (Aspect_Drawable)quick_item_->window()->winId();
    }
    else
    {
        aNativeWin = 0;
    }
#endif

    Handle(OpenGl_Context) aGlCtx = new OpenGl_Context();
    bool contextInitialized = aGlCtx->Init(is_core_profile_);
    if (!contextInitialized && is_core_profile_)
    {
        is_core_profile_ = false;
        contextInitialized = aGlCtx->Init(is_core_profile_);
    }

    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(view_->Window());
    if (!aWindow.IsNull())
    {
        std::cout << "[initializeGL] aWindow is not null, setting native window handle\n";
    }
    else
    {
        std::cout << "[initializeGL] aWindow is null, creating new window\n";
        aWindow = new Aspect_NeutralWindow();
        aWindow->SetVirtual(true);
    }

    aWindow->SetNativeHandle(aNativeWin);
    aWindow->SetSize(aViewSize.x(), aViewSize.y());
    view_->SetWindow(aWindow, aGlCtx->RenderingContext());

    // Display viewcube after window is set
    context_->Display(view_cube_, 0, 0, false);
}

void OCCRenderer::handleMousePressEvent(QMouseEvent *event)
{
    if (view_.IsNull() || myToAskNextFrame)
        return;

    const Graphic3d_Vec2i aClickPos(static_cast<int>(event->position().x() * scale_),
                                    static_cast<int>(event->position().y() * scale_));
    const Aspect_VKeyFlags aFlags = OcctGlTools::qtMouseModifiers2VKeys(event->modifiers());
    const Aspect_VKeyMouse aButton = OcctGlTools::qtMouseButtons2VKeys(event->button());

    if (UpdateMouseButtons(aClickPos, aButton, aFlags, false))
    {
        std::cout << "UpdateMouseButtons success" << std::endl;
    }
}

void OCCRenderer::handleMouseReleaseEvent(QMouseEvent *event)
{
    if (view_.IsNull() || myToAskNextFrame)
        return;

    const Graphic3d_Vec2i aClickPos(static_cast<int>(event->position().x() * scale_),
                                    static_cast<int>(event->position().y() * scale_));
    const Aspect_VKeyFlags aFlags = OcctGlTools::qtMouseModifiers2VKeys(event->modifiers());
    const Aspect_VKeyMouse aButtons = OcctGlTools::qtMouseButtons2VKeys(event->buttons());
    std::ignore = UpdateMouseButtons(aClickPos, aButtons, aFlags, true);
}

void OCCRenderer::handleMouseMoveEvent(QMouseEvent *event)
{
    if (view_.IsNull() || myToAskNextFrame)
        return;

    const Graphic3d_Vec2i aNewPos(static_cast<int>(event->position().x() * scale_),
                                  static_cast<int>(event->position().y() * scale_));
    std::ignore =
        UpdateMousePosition(aNewPos, PressedMouseButtons(),
                            OcctGlTools::qtMouseModifiers2VKeys(event->modifiers()), false);
}

void OCCRenderer::handleHoverMoveEvent(QHoverEvent *event)
{
    if (view_.IsNull() || myToAskNextFrame)
        return;

    const Graphic3d_Vec2i aNewPos(static_cast<int>(event->position().x() * scale_),
                                  static_cast<int>(event->position().y() * scale_));
    std::ignore =
        UpdateMousePosition(aNewPos, Aspect_VKeyMouse_NONE,
                            OcctGlTools::qtMouseModifiers2VKeys(event->modifiers()), false);
}

void OCCRenderer::fitAll()
{
    if (!view_.IsNull())
    {
        pending_fit_all_ = true;
    }
}

void OCCRenderer::handleWheelEvent(QWheelEvent *event)
{
    if (view_.IsNull() || myToAskNextFrame)
        return;

    const Graphic3d_Vec2i aPos(static_cast<int>(event->position().x() * scale_),
                               static_cast<int>(event->position().y() * scale_));

    const double aDelta = double(event->angleDelta().y()) / 8.0;

    std::ignore = UpdateZoom(Aspect_ScrollDelta(aPos, aDelta));
}

void OCCRenderer::setViewCubeSize(double size)
{
    if (!view_cube_.IsNull())
    {
        view_cube_->SetSize(size);
        if (!context_.IsNull())
        {
            context_->Update(view_cube_, false);
        }
    }
}

void OCCRenderer::setViewCubePosition(int x, int y)
{
    if (!view_cube_.IsNull())
    {
        Handle(Graphic3d_TransformPers) transform = view_cube_->TransformPersistence();
        if (!transform.IsNull())
        {
            transform->SetOffset2d(Graphic3d_Vec2i(x, y));
            if (!context_.IsNull())
            {
                context_->Update(view_cube_, false);
            }
        }
    }
}

void OCCRenderer::handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                                   const Handle(V3d_View) & theView)
{
    AIS_ViewController::handleViewRedraw(theCtx, theView);

    if (myToAskNextFrame && quick_item_)
    {
        QMetaObject::invokeMethod(const_cast<OccViewerItem *>(quick_item_), "update",
                                  Qt::QueuedConnection);
    }
}
} // namespace geotoys
