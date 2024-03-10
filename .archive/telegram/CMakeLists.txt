project(telegram VERSION 0.0)



albert_plugin(
    SOURCE_FILES
        src/plugin.cpp
        src/plugin.h
    PRIVATE_LINK_LIBRARIES
        Qt6::Concurrent
        Td::TdStatic
)

add_subdirectory(td)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
