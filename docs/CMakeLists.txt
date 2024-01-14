#if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

project(docs VERSION 6.0)

find_package(Qt6 REQUIRED COMPONENTS Sql Network)
find_package(LibArchive REQUIRED)

albert_plugin(
    SOURCE_FILES
        src/configwidget.ui
        src/plugin.cpp
        src/plugin.h
    PRIVATE_LINK_LIBRARIES
        Qt6::Sql
        Qt6::Network
        LibArchive::LibArchive
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)

#endif()
