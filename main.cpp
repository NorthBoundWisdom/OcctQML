#include <QtQml>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickView>
#include <QQuickWindow>

#include "OccViewerItem.h"

int main(int argc, char *argv[])
{
#if defined(_WIN32)
    // never use ANGLE on Windows, since OCCT 3D Viewer does not expect this
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#elif defined(__APPLE__)
    qputenv("QT_QPA_PLATFORM", "cocoa");
#else
    // wayland-egl, minimal, xcb, vnc, linuxfb, eglfs, minimalegl, offscreen,
    // wayland, vkkhrdisplay.
    qputenv("QT_QPA_PLATFORM", "xcb");

    // WSL2-specific workarounds
    const char *wslEnv = getenv("WSL_DISTRO_NAME");
    if (wslEnv != nullptr)
    {
        // Force software OpenGL in WSL2 if D3D12 causes issues
        // qputenv("LIBGL_ALWAYS_SOFTWARE", "1");

        // Disable VSync which can cause issues in WSL2
        qputenv("vblank_mode", "0");
        qputenv("__GL_SYNC_TO_VBLANK", "0");

        // Force OpenGL 3.3 compatibility profile for better WSL2 support
        qputenv("MESA_GL_VERSION_OVERRIDE", "3.3COMPAT");
        qputenv("MESA_GLSL_VERSION_OVERRIDE", "330");
    }
#endif

    // Set up OpenGL surface format BEFORE creating QGuiApplication
    QSurfaceFormat aGlFormat;
    aGlFormat.setDepthBufferSize(24);
    aGlFormat.setStencilBufferSize(8);

// Use compatibility profile and fallback version for WSL2
#if defined(_WIN32)
    aGlFormat.setVersion(4, 5);
    aGlFormat.setProfile(QSurfaceFormat::CoreProfile);
#else
    // Use a more compatible version for WSL2/D3D12
    aGlFormat.setVersion(3, 3);
    aGlFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
#endif

    QSurfaceFormat::setDefaultFormat(aGlFormat);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);

    // register custom QML type
    qmlRegisterType<geotoys::OccViewerItem>("OcctQML", 1, 0, "OccViewerItem");

    // use QQuickView to create window and load QML
    QQuickView view;
    view.setTitle("QML Occ Demo");
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.resize(800, 600);

    // load QML file
    view.setSource(QUrl("qrc:/main.qml"));

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
