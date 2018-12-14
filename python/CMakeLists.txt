cmake_minimum_required(VERSION 3.1.3)

PROJECT(python)

FILE(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Widgets)
add_subdirectory(pybind11)

add_library(${PROJECT_NAME} SHARED ${SRC} ${PROJECT_NAME}.qrc)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt5::Widgets
    PRIVATE
        pybind11::embed
        albertcore
        xdg
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins)
install(DIRECTORY "share/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/org.albert.extension.python")
