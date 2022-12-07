cmake_minimum_required(VERSION 3.1.3)

project(firefoxbookmarks)

file(GLOB_RECURSE SRC src/*)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Widgets Concurrent Sql)

add_library(${PROJECT_NAME} SHARED ${SRC} ${PROJECT_NAME}.qrc metadata.json)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Concurrent
        Qt5::Sql
        Qt5::Widgets
        albert::lib
        xdg
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins)
