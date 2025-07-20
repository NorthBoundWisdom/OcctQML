set(OCC_QWIDGET_SOURCES
    MainWindow.cpp
    OcctGlTools.cpp
    OcctQtViewer.cpp
    main.cpp
    MainWindow.h
    OcctFrameBuffer.h
    OcctGlTools.h
    OcctQtViewer.h
)

add_executable(OccQWidget ${OCC_QWIDGET_SOURCES})

target_link_directories(OccQWidget PRIVATE
    ${Qt_LIBRARY_DIR}
    ${OpenCASCADE_LIBRARY_DIR}
)

target_include_directories(OccQWidget PRIVATE
    ${Qt_INCLUDE_DIR}
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(OccQWidget PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::Gui
    ${OpenCASCADE_LIBRARIES}
    OpenGL::GL
    )

deploy_qt_dependencies(
    TARGET_NAME OccQWidget
)
