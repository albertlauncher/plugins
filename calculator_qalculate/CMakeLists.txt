project(calculator_qalculate VERSION 1.3)
albert_plugin(
    NAME Calculator
    DESCRIPTION "Evaluate math expressions using Qalculate"
    LONG_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets
    LIB_DEPENDENCIES libqalculate
    CREDITS "qalculate GPL-2"
)

find_package(PkgConfig)
pkg_check_modules(LIBQALCULATE REQUIRED libqalculate)
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBQALCULATE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBQALCULATE_LIBRARIES})
target_link_directories(${PROJECT_NAME} PRIVATE ${LIBQALCULATE_LIBRARY_DIRS})
