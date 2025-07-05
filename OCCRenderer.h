#ifndef OCCRENDER_NEW_H
#define OCCRENDER_NEW_H

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>
#include <QWheelEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewController.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Aspect_VKey.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <Standard_Handle.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include "OccSceneManager.h"

namespace geotoys
{

class OccViewerItem;

// 鼠标事件类型枚举
enum class MouseEventType
{
    Press,
    Release,
    Move,
    Hover
};

class OCCRenderer : public QQuickFramebufferObject::Renderer,
                    public AIS_ViewController
{
public:
    OCCRenderer(const OccViewerItem *item);
    ~OCCRenderer() override;

    void render() override;
    void synchronize(QQuickFramebufferObject *item) override;

    QOpenGLFramebufferObject *
    createFramebufferObject(const QSize &size) override;

    OccSceneManager *getSceneManager() const
    {
        return scene_manager_;
    }

    void handleWheelEvent(QWheelEvent *event);
    void handleMousePressEvent(QMouseEvent *event);
    void handleMouseReleaseEvent(QMouseEvent *event);
    void handleMouseMoveEvent(QMouseEvent *event);
    void handleHoverMoveEvent(QHoverEvent *event);

    void fitAll();

protected:
    //! Handle view redraw for animation support
    void handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                          const Handle(V3d_View) & theView) override;

private:
    void initializeGL(const QSize &size);
    void setViewCubeSize(double size);
    void setViewCubePosition(int x, int y);

private:
    const OccViewerItem *quick_item_ = nullptr;
    bool is_core_profile_ = false;

    Handle(V3d_Viewer) viewer_;
    Handle(V3d_View) view_;
    Handle(AIS_ViewCube) view_cube_;
    Handle(AIS_InteractiveContext) context_;
    OccSceneManager *scene_manager_;
    double scale_ = 1.0;
    bool pending_fit_all_ = false;
};
} // namespace geotoys
#endif // OCCRENDER_H
