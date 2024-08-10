cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(ssh VERSION 8.4)

albert_plugin(
    INCLUDE PRIVATE $<TARGET_PROPERTY:albert::applications,INTERFACE_INCLUDE_DIRECTORIES>
    QT Widgets
)
