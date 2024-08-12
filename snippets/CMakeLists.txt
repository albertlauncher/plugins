cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(snippets VERSION 6.0)

albert_plugin(
    INCLUDE
        INTERFACE include
        PRIVATE include/albert/plugin
    QT
        Concurrent Widgets
)
