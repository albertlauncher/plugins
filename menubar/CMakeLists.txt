if (APPLE)

cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(menubar VERSION 1.4)

find_library(CORESERVICES_LIBRARY CoreServices)

albert_plugin(
    LINK PRIVATE
        ${CORESERVICES_LIBRARY}
         objc
         "-framework ApplicationServices"
         "-framework Cocoa"
    QT
        Widgets
)

endif()
