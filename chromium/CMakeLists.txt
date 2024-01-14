project(chromium VERSION 6.0)

find_package(Qt6 REQUIRED COMPONENTS Concurrent)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/configwidget.ui
        src/plugin.cpp
        src/plugin.h
    PRIVATE_LINK_LIBRARIES
        Qt6::Concurrent
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)
