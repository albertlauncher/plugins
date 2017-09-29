cmake_minimum_required(VERSION 3.1.3)

project(chromebookmarks)

file(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Widgets Concurrent)

add_library(${PROJECT_NAME} SHARED ${SRC} ${PROJECT_NAME}.qrc)

target_include_directories(${PROJECT_NAME} PRIVATE src/)

target_link_libraries(${PROJECT_NAME}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    albertcore
    xdg
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/albert/plugins)
