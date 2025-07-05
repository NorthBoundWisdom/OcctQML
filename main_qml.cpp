#include <QtQml>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickView>
#include <QQuickWindow>

#include "OccQuickItem.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
    QGuiApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);

    // register custom QML type
    qmlRegisterType<geotoys::OccQuickItem>("GeoToys", 1, 0, "OccQuickItem");

    // use QQuickView to create window and load QML
    QQuickView view;
    view.setTitle("QML Demo - C++ created window");
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    // set window properties
    view.resize(800, 600);

    // load QML file
    view.setSource(QUrl("qrc:/main_qml.qml"));

    // show window
    view.show();

    // check if loaded successfully
    if (view.status() == QQuickView::Error)
    {
        qWarning("Failed to load QML file!");
        return -1;
    }

    return app.exec();
}
