project(calculator_qalculate VERSION 1.2)
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
    target_link_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/lib)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE qalculate)

