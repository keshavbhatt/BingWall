cmake_minimum_required(VERSION 3.21)

project(bingwall
    VERSION 3.1
    DESCRIPTION "Bing Wallpaper of the day for your desktop"
    HOMEPAGE_URL "https://github.com/keshavbhatt/BingWall"
    LANGUAGES CXX
)

set(APP_NAME "BingWall")
set(APP_EXECUTABLE_NAME "bing-wall")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/bingwall.desktop.in"
    "${CMAKE_CURRENT_BINARY_DIR}/bingwall.desktop"
    @ONLY
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Developed and shipped against Qt 6.11 (kde-qt6-core24-sdk / kf6-core24).
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    DBus
)

qt_standard_project_setup()

# Dev builds against the kde-qt6-core24-sdk snap on a non-Ubuntu host: some
# imported Qt targets export the SDK's generic /usr/include (Ubuntu glibc
# headers), which shadow the host toolchain's libc headers and break
# libstdc++. Strip that one directory from every imported target; the Qt
# headers proper live under .../include/<arch>/qt6 and are unaffected.
# Harmless in snapcraft builds (the dir then equals the real sysroot's).
if(CMAKE_PREFIX_PATH MATCHES "/snap/kde-qt6-core24-sdk/")
    get_property(_bingwall_imported DIRECTORY PROPERTY IMPORTED_TARGETS)
    foreach(_tgt IN LISTS _bingwall_imported)
        get_target_property(_incs ${_tgt} INTERFACE_INCLUDE_DIRECTORIES)
        if(_incs)
            set(_filtered "")
            foreach(_dir IN LISTS _incs)
                if(NOT _dir MATCHES "/snap/kde-qt6-core24-sdk/[^/]+/usr/include$")
                    list(APPEND _filtered "${_dir}")
                endif()
            endforeach()
            set_property(TARGET ${_tgt} PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES "${_filtered}")
        endif()
    endforeach()
endif()

# Suppress qDebug output in non-debug builds.
add_compile_definitions(
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:QT_NO_DEBUG_OUTPUT>
)

# Plain add_executable (not qt_add_executable): the app has no QML, and
# skipping Qt target finalization avoids running the SDK snap's
# qmlimportscanner, which cannot start on non-Ubuntu dev hosts.
add_executable(${APP_EXECUTABLE_NAME}
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/controlbutton.cpp
    src/controlbutton.h
    src/darkstyle.cpp
    src/darkstyle.h
    src/gridlayoututil.h
    src/remotepixmaplabel2.cpp
    src/remotepixmaplabel2.h
    src/request.cpp
    src/request.h
    src/scrolltext.cpp
    src/scrolltext.h
    src/utils.cpp
    src/utils.h
    src/waitingspinnerwidget.cpp
    src/waitingspinnerwidget.h
    src/wallpapersetter.cpp
    src/wallpapersetter.h
    src/action_ui.ui
    src/downloaded.ui
    src/downloaded_option.ui
    src/listitem_ui.ui
    src/settings.ui
    src/darkstyle.qrc
    src/images.qrc
)

# uic-generated headers include promoted-widget headers by bare name.
target_include_directories(${APP_EXECUTABLE_NAME} PRIVATE src)

target_compile_definitions(${APP_EXECUTABLE_NAME} PRIVATE
    APPNAMESTR="${APP_NAME}"
    VERSIONSTR="${PROJECT_VERSION}"
)

target_link_libraries(${APP_EXECUTABLE_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::DBus
)

# Install rules
include(GNUInstallDirs)
install(TARGETS ${APP_EXECUTABLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

if(NOT WIN32)
    install(FILES src/icons/bingwall.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps
    )
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bingwall.desktop"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()
