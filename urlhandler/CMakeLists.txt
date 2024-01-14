project(urlhandler VERSION 5.0)

find_package(Qt6 REQUIRED COMPONENTS Gui)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/plugin.cpp
        src/plugin.h
    PRIVATE_LINK_LIBRARIES
        Qt6::Gui  # For DesktopServices
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)
