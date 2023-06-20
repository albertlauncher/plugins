project(python VERSION 1.7)
albert_plugin(
    NAME "Python"
    DESCRIPTION "Load Python plugins"
    LONG_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets
    CREDITS "pybind11 BSD-3-Clause"
)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11)
target_link_libraries(${PROJECT_NAME} PRIVATE pybind11::embed)
install(DIRECTORY "plugins/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/plugins")
