cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(spotify VERSION 1.0.0)

file(GLOB_RECURSE SOURCES "src/*")

albert_plugin(QT Widgets)
