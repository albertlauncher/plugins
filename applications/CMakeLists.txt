cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(applications VERSION 10.0)

albert_plugin(
    SOURCES
        src/plugin.h
    I18N_SOURCES
        src/*.h
        src/*.cpp
        src/*.mm
        src/*.ui
)

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
    target_sources(${PROJECT_NAME} PRIVATE src/plugin_mac.mm)
    find_package(Qt6 6.0 REQUIRED COMPONENTS Core)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
elseif(UNIX)
    target_sources(${PROJECT_NAME} PRIVATE src/configwidget_xdg.ui src/plugin_xdg.cpp)
    find_package(Qt6 6.0 REQUIRED COMPONENTS Widgets Concurrent)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Concurrent)
endif()

