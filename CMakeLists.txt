cmake_minimum_required(VERSION 3.24)

project(
    GeoToys
    VERSION 0.9.7
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core OpenGLWidgets Quick Widgets OpenGL Gui Qml)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
include(DeployQt)

find_package(OpenCASCADE REQUIRED)

set(OCC_QML_SOURCES
    main.cpp
    OccViewerItem.cpp
    OCCRenderer.cpp
    OcctGlTools.cpp
    OccSceneManager.cpp
)

set(OCC_QML_HEADERS
    OccViewerItem.h
    OCCRenderer.h
    OcctFrameBuffer.h
    OcctGlTools.h
    OccSceneManager.h
)

set(OCC_QML_RESOURCES
    main.qrc
)

add_executable(OccQml
    ${OCC_QML_SOURCES}
    ${OCC_QML_HEADERS}
    ${OCC_QML_RESOURCES}
)

target_link_directories(OccQml PRIVATE
    ${Qt_LIBRARY_DIR}
    ${OpenCASCADE_LIBRARY_DIR}
)

target_include_directories(OccQml PRIVATE
    ${Qt_INCLUDE_DIR}
    ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(OccQml PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Gui
    Qt6::Qml
    OpenGL::GL
    ${OpenCASCADE_LIBRARIES}
    )

deploy_qt_dependencies(
    TARGET_NAME OccQml
    QML_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)

# add subdir if exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/OccQWidget)
    add_subdirectory(OccQWidget)
endif()
