#include "OccViewerItem.h"

#include <random>

#include <QColor>
#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

#include <BRepPrimAPI_MakeBox.hxx>

#include "OCCRenderer.h"
#include "OccSceneManager.h"

namespace geotoys
{

OccViewerItem::OccViewerItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , visible_(true)
{
    setMirrorVertically(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFlag(QQuickItem::ItemAcceptsInputMethod, true);
    setFlag(QQuickItem::ItemIsFocusScope, true);
    setFocus(true);
}

OccViewerItem::~OccViewerItem()
{
}

QQuickFramebufferObject::Renderer *OccViewerItem::createRenderer() const
{
    renderer_ = new OCCRenderer(const_cast<OccViewerItem *>(this));
    return renderer_;
}

void OccViewerItem::setWindowVisible(bool visible)
{
    if (visible_ != visible)
    {
        visible_ = visible;
        this->setVisible(visible);
        Q_EMIT windowVisibleChanged();
        update();
    }
}

void OccViewerItem::toggleWindow()
{
    setWindowVisible(!visible_);
}

OccSceneManager *OccViewerItem::getSceneManager() const
{
    return renderer_->getSceneManager();
}

bool OccViewerItem::addShape(const QString & /*id*/,
                             const QVariant & /*shapeData*/,
                             const QColor & /*color*/, bool /*display*/)
{
    qWarning()
        << "addShape not implemented yet - need to convert QVariant to TopoDS_Shape";
    return false;
}

bool OccViewerItem::removeShape(const QString &id)
{
    if (auto sceneManager = getSceneManager())
    {
        bool result = sceneManager->removeShape(id.toStdString());
        if (result)
        {
            // 强制更新渲染
            update();
        }
        return result;
    }
    return false;
}

bool OccViewerItem::updateShape(const QString & /*id*/,
                                const QVariant & /*shapeData*/)
{
    qWarning()
        << "updateShape not implemented yet - need to convert QVariant to TopoDS_Shape";
    return false;
}

bool OccViewerItem::setShapeColor(const QString &id, const QColor &color)
{
    if (auto sceneManager = getSceneManager())
    {
        Quantity_Color occColor(color.red(), color.green(), color.blue(),
                                Quantity_TOC_RGB);
        return sceneManager->setShapeColor(id.toStdString(), occColor);
    }
    return false;
}

QStringList OccViewerItem::getAllShapeIds() const
{
    if (auto sceneManager = getSceneManager())
    {
        std::vector<std::string> ids = sceneManager->getAllShapeIds();
        QStringList result;
        for (const auto &id : ids)
        {
            result.append(QString::fromStdString(id));
        }
        return result;
    }
    return QStringList();
}

void OccViewerItem::clearAllShapes()
{
    if (auto sceneManager = getSceneManager())
    {
        sceneManager->clearAllShapes();
        // 强制更新渲染
        update();
    }
}

void OccViewerItem::mousePressEvent(QMouseEvent *event)
{
    renderer_->handleMousePressEvent(event);
    update();
}

void OccViewerItem::mouseReleaseEvent(QMouseEvent *event)
{
    renderer_->handleMouseReleaseEvent(event);
    update();
}

void OccViewerItem::mouseMoveEvent(QMouseEvent *event)
{
    renderer_->handleMouseMoveEvent(event);
    update();
}

void OccViewerItem::wheelEvent(QWheelEvent *event)
{
    renderer_->handleWheelEvent(event);
    update();
}

void OccViewerItem::hoverMoveEvent(QHoverEvent *event)
{
    if (event->position().toPoint() == last_mouse_pos_)
    {
        return;
    }

    last_mouse_pos_ = event->position().toPoint();
    renderer_->handleHoverMoveEvent(event);
    update();
}

void OccViewerItem::fitAll()
{
    renderer_->fitAll();
    update();
}

void OccViewerItem::addTestShape()
{
    auto sceneManager = getSceneManager();
    if (!sceneManager)
    {
        return;
    }

    TopoDS_Shape aBox = BRepPrimAPI_MakeBox(100.0, 50.0, 90.0).Shape();
    sceneManager->addShape("test_box", aBox, Quantity_NOC_YELLOW, true);
    renderer_->fitAll();
    update();
}

void OccViewerItem::removeTestShape()
{
    auto sceneManager = getSceneManager();
    if (!sceneManager)
    {
        return;
    }
    sceneManager->removeShape("test_box");

    renderer_->fitAll();
    update();
}

void OccViewerItem::updateTestShape()
{
    // change size and color
    auto sceneManager = getSceneManager();
    if (!sceneManager)
    {
        return;
    }
    // get rand size and color
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double size = dis(gen) * 100 + 1;
    double color = dis(gen);
    double rand_scale = dis(gen);
    TopoDS_Shape aBox =
        BRepPrimAPI_MakeBox(size, rand_scale * size, rand_scale * size).Shape();
    sceneManager->updateShape("test_box", aBox);
    sceneManager->setShapeColor(
        "test_box", Quantity_Color(color, rand_scale * color,
                                   rand_scale * color, Quantity_TOC_RGB));
    renderer_->fitAll();
    update();
}
} // namespace geotoys
