#if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

project(mpris VERSION 3.1)

find_package(Qt6 REQUIRED COMPONENTS DBus)

albert_plugin(
    SOURCE_FILES
        src/configwidget.ui
        src/plugin.cpp
        src/plugin.h
    PRIVATE_LINK_LIBRARIES
        Qt6::DBus
)

#endif()
