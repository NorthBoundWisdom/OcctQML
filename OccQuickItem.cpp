#include "OccQuickItem.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>

#include "OCCRenderer.h"

namespace geotoys
{

OccQuickItem::OccQuickItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , m_visible(true)
    , m_title("OCC QML Viewer")
{
    setMirrorVertically(true); // OCC的Y轴与QML相反

    // 使用更长的更新间隔来减少性能开销
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() { update(); });
    timer->start(2000);
}

OccQuickItem::~OccQuickItem() {}

QQuickFramebufferObject::Renderer *OccQuickItem::createRenderer() const
{
    qDebug() << "OccQuickItem::createRenderer()";
    return new OCCRenderer(const_cast<OccQuickItem *>(this));
}

void OccQuickItem::setWindowVisible(bool visible)
{
    if (m_visible != visible)
    {
        m_visible = visible;
        Q_EMIT windowVisibleChanged();
        update();
    }
}

void OccQuickItem::setWindowTitle(const QString &title)
{
    if (m_title != title)
    {
        m_title = title;
        Q_EMIT windowTitleChanged();
        update();
    }
}

void OccQuickItem::showWindow() { setWindowVisible(true); }

void OccQuickItem::hideWindow() { setWindowVisible(false); }

void OccQuickItem::toggleWindow() { setWindowVisible(!m_visible); }
} // namespace geotoys
