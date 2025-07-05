# On windows, if qt is installed by qt installer, the dlls will not be copied to the target directory automatically.
# So we need to use windeployqt to deploy the dlls to the target directory.
# If you use vcpkg, this can be done automatically. Mac or Linux can ignore this file.

#[[
Usage   :
include(cmake/DeployQt.cmake)
deploy_qt_dependencies(TARGET_NAME your_target_name [QML_DIR path/to/qml])

Parameters:
    - TARGET_NAME: Required, the name of the target executable file
    - QML_DIR: Optional, the path to the QML source directory, used for QML module dependency detection
]]

function(deploy_qt_dependencies)
    # Parse function parameters
    set(options "")
    set(oneValueArgs TARGET_NAME QML_DIR)
    set(multiValueArgs "")
    cmake_parse_arguments(DEPLOY_QT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEPLOY_QT_TARGET_NAME)
        message(FATAL_ERROR "deploy_qt_dependencies: TARGET_NAME is required")
    endif()

    if(NOT TARGET ${DEPLOY_QT_TARGET_NAME})
        message(FATAL_ERROR "deploy_qt_dependencies: Target '${DEPLOY_QT_TARGET_NAME}' does not exist")
    endif()

    if(NOT DEPLOY_QT_QML_DIR)
        set(DEPLOY_QT_QML_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(WIN32)
        _deploy_qt_windows(${DEPLOY_QT_TARGET_NAME} ${DEPLOY_QT_QML_DIR})
    elseif(APPLE)
        _deploy_qt_macos(${DEPLOY_QT_TARGET_NAME} ${DEPLOY_QT_QML_DIR})
    elseif(UNIX)
        _deploy_qt_linux(${DEPLOY_QT_TARGET_NAME} ${DEPLOY_QT_QML_DIR})
    else()
        message(WARNING "deploy_qt_dependencies: Unsupported platform for Qt deployment")
    endif()
endfunction()

# Windows Qt deployment
function(_deploy_qt_windows target_name qml_dir)
    # Get Qt qmake executable path
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    if(NOT _qmake_executable)
        message(WARNING "deploy_qt_dependencies: Could not find Qt6::qmake, skipping Qt deployment")
        return()
    endif()

    message(STATUS "Qt6 qmake executable: ${_qmake_executable}")
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

    # Clear previous windeployqt cache
    unset(WINDEPLOYQT_EXECUTABLE CACHE)

    # Find windeployqt in Qt bin directory
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
        HINTS "${_qt_bin_dir}"
        NO_DEFAULT_PATH
    )
    message(STATUS "windeployqt executable: ${WINDEPLOYQT_EXECUTABLE}")

    if(NOT WINDEPLOYQT_EXECUTABLE)
        message(FATAL_ERROR "Could not find windeployqt in Qt6 directory: ${_qt_bin_dir}")
    endif()

    # Create Qt dependency deployment target
    set(deploy_target_name "Deploy_Qt_${target_name}")
    add_custom_target(${deploy_target_name}
        COMMENT "Deploying Qt dependencies for ${target_name}"
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --qmldir "${qml_dir}"
            --no-translations
            --compiler-runtime
            \"$<TARGET_FILE:${target_name}>\"
        COMMENT "Qt dependencies deployed successfully"
        DEPENDS ${target_name}
    )

    # Set target properties for external reference
    set_target_properties(${deploy_target_name} PROPERTIES
        FOLDER "Deployment"
        DEPLOY_QT_TARGET "${target_name}"
    )

    message(STATUS "Created Qt deployment target: ${deploy_target_name}")
endfunction()

# macOS Qt deployment
function(_deploy_qt_macos target_name qml_dir)
    # Add macOS Qt deployment
endfunction()

# Linux Qt deployment (usually no special deployment)
function(_deploy_qt_linux target_name qml_dir)
    # Add Linux Qt deployment
endfunction()

# Helper function: Check Qt version compatibility
function(check_qt_version_compatibility)
    if(NOT Qt6_FOUND)
        message(FATAL_ERROR "deploy_qt_dependencies: Qt6 is required")
    endif()

    # Check minimum Qt version
    if(Qt6_VERSION VERSION_LESS "6.0.0")
        message(FATAL_ERROR "deploy_qt_dependencies: Qt 6.0.0 or higher is required")
    endif()

    message(STATUS "Using Qt version: ${Qt6_VERSION}")
endfunction()

# Helper function: Get all Qt deployment targets
function(get_qt_deploy_targets output_var)
    get_property(all_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
    set(deploy_targets "")

    foreach(target ${all_targets})
        get_target_property(is_deploy_target ${target} DEPLOY_QT_TARGET)
        if(is_deploy_target)
            list(APPEND deploy_targets ${target})
        endif()
    endforeach()

    set(${output_var} ${deploy_targets} PARENT_SCOPE)
endfunction()
