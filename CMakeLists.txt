cmake_minimum_required(VERSION 3.24)

project(
    GeoToys
    VERSION 0.9.7
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Quick OpenGL Gui Qml)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(OpenCASCADE REQUIRED)

set(OCC_QML_SOURCES
    main_qml.cpp
    OccQuickItem.cpp
    OCCRenderer.cpp
    OcctGlTools.cpp
)

set(OCC_QML_HEADERS
    OccQuickItem.h
    OCCRenderer.h
    OcctFrameBuffer.h
    OcctGlTools.h
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
    Qt6::OpenGL
    Qt6::Gui
    Qt6::Qml
    ${OpenCASCADE_LIBRARIES}
    )
