cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(applications VERSION 12.4)

file(GLOB_RECURSE I18N_SOURCES src/*)

albert_plugin(
    INCLUDE
        INTERFACE include
        PRIVATE include/albert/plugin src
    SOURCES
        src/applicationbase.cpp
        src/applicationbase.h
        src/pluginbase.cpp
        src/pluginbase.h
        include/albert/plugin/${PROJECT_NAME}.h
    I18N_SOURCES
        ${I18N_SOURCES}
    QT
        Widgets
        Concurrent
)

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
    target_include_directories(${PROJECT_NAME} PRIVATE src/mac)
    target_sources(${PROJECT_NAME} PRIVATE
        src/mac/configwidget.ui
        src/mac/application.mm
        src/mac/application.h
        src/mac/plugin.cpp
        src/mac/plugin.h
        src/mac/terminal.cpp
        src/mac/terminal.h
    )
elseif(UNIX)
    target_include_directories(${PROJECT_NAME} PRIVATE src/xdg)
    target_sources(${PROJECT_NAME} PRIVATE
        src/xdg/application.cpp
        src/xdg/application.h
        src/xdg/configwidget.ui
        src/xdg/desktopentryparser.cpp
        src/xdg/desktopentryparser.h
        src/xdg/plugin.cpp
        src/xdg/plugin.h
        src/xdg/terminal.cpp
        src/xdg/terminal.h
    )
endif()

