cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(applications VERSION 11.0)

albert_plugin(
    SOURCES
        src/plugin.cpp
        src/plugin.h
    I18N_SOURCES
        src/*.h
        src/*.cpp
        src/*.mm
        src/*.ui
    QT
        Widgets
        Concurrent
)

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
    target_sources(${PROJECT_NAME} PRIVATE
        src/configwidget_mac.ui
        src/plugin_mac.mm
    )
elseif(UNIX)
    target_sources(${PROJECT_NAME} PRIVATE
        src/configwidget_xdg.ui
        src/plugin_xdg.cpp
    )
endif()

