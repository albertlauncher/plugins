project(ssh VERSION 8.0)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/configwidget.ui
        src/plugin.cpp
        src/plugin.h
)
