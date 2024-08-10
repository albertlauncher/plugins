cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(path VERSION 1.1)

albert_plugin(
    INCLUDE PRIVATE $<TARGET_PROPERTY:albert::applications,INTERFACE_INCLUDE_DIRECTORIES>
    QT Concurrent
)
