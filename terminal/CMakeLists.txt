project(terminal VERSION 5.0)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/plugin.cpp
        src/plugin.h
    PRIVATE_LINK_LIBRARIES
        Qt6::Concurrent
)
