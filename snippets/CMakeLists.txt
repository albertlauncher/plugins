cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(snippets VERSION 5.3)

albert_plugin(
    INCLUDE
        INTERFACE include
        PRIVATE include/albert/snippets
    QT
        Concurrent Widgets
)
