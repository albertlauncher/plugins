project(clipboard VERSION 3.0)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/plugin.cpp
        src/plugin.h
    PRIVATE_INCLUDE_DIRECTORIES
        $<TARGET_PROPERTY:albert::snippets,INTERFACE_INCLUDE_DIRECTORIES>
)


