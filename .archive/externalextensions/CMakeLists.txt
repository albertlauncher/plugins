cmake_minimum_required(VERSION 3.1.3)

project(externalextensions)

file(GLOB_RECURSE SRC src/*)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Widgets)

add_library(${PROJECT_NAME} SHARED ${SRC} metadata.json)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Widgets
        albert::lib
        xdg
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins)
