#ifndef QMLOCCVIEWER_H
#define QMLOCCVIEWER_H

#include <QColor>
#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>
#include <QVariant>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <Standard_Handle.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include "OccSceneManager.h"

namespace geotoys
{
class OCCRenderer;

class OccViewerItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(bool windowVisible READ windowVisible WRITE setWindowVisible
                   NOTIFY windowVisibleChanged)

public:
    OccViewerItem(QQuickItem *parent = nullptr);
    ~OccViewerItem() override;

    Renderer *createRenderer() const override;

    bool windowVisible() const
    {
        return visible_;
    }
    void setWindowVisible(bool visible);

    Q_INVOKABLE void toggleWindow();

    Q_INVOKABLE bool addShape(const QString &id, const QVariant &shapeData,
                              const QColor &color, bool display);
    Q_INVOKABLE bool removeShape(const QString &id);
    Q_INVOKABLE bool updateShape(const QString &id, const QVariant &shapeData);
    Q_INVOKABLE bool setShapeColor(const QString &id, const QColor &color);
    Q_INVOKABLE QStringList getAllShapeIds() const;
    Q_INVOKABLE void clearAllShapes();
    Q_INVOKABLE void fitAll();

    // for test
    Q_INVOKABLE void addTestShape();
    Q_INVOKABLE void removeTestShape();
    Q_INVOKABLE void updateTestShape();

protected:
    OccSceneManager *getSceneManager() const;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;

Q_SIGNALS:
    void windowVisibleChanged();

private:
    bool visible_;
    QPoint last_mouse_pos_;

    mutable OCCRenderer *renderer_ = nullptr;
};
} // namespace geotoys

#endif // QMLOCCVIEWER_H
