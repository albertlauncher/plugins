cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(chromium VERSION 7.0)

albert_plugin(QT Widgets Concurrent)
