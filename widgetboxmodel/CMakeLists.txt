cmake_minimum_required(VERSION 3.1.3)

project(widgetboxmodel)

file(GLOB_RECURSE SRC src/*)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Svg Widgets X11Extras)

add_library(${PROJECT_NAME} SHARED ${SRC} metadata.json)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

find_package(Qt5 5.5.0 REQUIRED X11Extras)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Svg
        Qt5::Widgets
        Qt5::X11Extras
        albert::lib
        Xext
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins)
install(DIRECTORY "share/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/org.albert.frontend.widgetboxmodel")
