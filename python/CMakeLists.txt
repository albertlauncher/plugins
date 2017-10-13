cmake_minimum_required(VERSION 3.1.3)

PROJECT(python)

FILE(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Widgets)
add_subdirectory(pybind11)

add_library(${PROJECT_NAME} SHARED ${SRC} ${PROJECT_NAME}.qrc)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        ${Qt5Widgets_LIBRARIES}
    PRIVATE
        pybind11::embed
        albertcore
        xdg
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert/plugins)
