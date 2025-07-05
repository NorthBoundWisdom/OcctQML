#ifndef QMLOCCVIEWER_H
#define QMLOCCVIEWER_H

#include <QOpenGLFramebufferObject>
#include <QQuickFramebufferObject>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <Standard_Handle.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

namespace geotoys
{
class OccQuickItem : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(
        bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle NOTIFY windowTitleChanged)

public:
    OccQuickItem(QQuickItem *parent = nullptr);
    ~OccQuickItem() override;

    Renderer *createRenderer() const override;

    bool windowVisible() const { return m_visible; }
    void setWindowVisible(bool visible);

    QString windowTitle() const { return m_title; }
    void setWindowTitle(const QString &title);

    Q_INVOKABLE void showWindow();
    Q_INVOKABLE void hideWindow();
    Q_INVOKABLE void toggleWindow();

Q_SIGNALS:
    void windowVisibleChanged();
    void windowTitleChanged();

private:
    bool m_visible;
    QString m_title;
};
} // namespace geotoys

#endif // QMLOCCVIEWER_H
