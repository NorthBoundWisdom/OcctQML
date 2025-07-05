#ifndef OCCRENDER_NEW_H
#define OCCRENDER_NEW_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewController.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <Standard_Handle.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

namespace geotoys
{

class OccQuickItem;

class OCCRenderer : public QQuickFramebufferObject::Renderer, public AIS_ViewController
{
public:
    OCCRenderer(OccQuickItem *item);
    ~OCCRenderer() override;

    void render() override;
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

private:
    void initializeGL(const QSize &size);

private:
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;

    bool myIsCoreProfile = false;
};
} // namespace geotoys
#endif // OCCRENDER_H
