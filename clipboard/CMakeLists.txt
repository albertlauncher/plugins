cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(clipboard VERSION 3.3)

albert_plugin(
    INCLUDE PRIVATE $<TARGET_PROPERTY:albert::snippets,INTERFACE_INCLUDE_DIRECTORIES>
    QT Widgets
)
