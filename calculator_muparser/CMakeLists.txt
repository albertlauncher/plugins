project(calculator_muparser VERSION 1.6)
albert_plugin(
    NAME Calculator
    DESCRIPTION "Evaluate math expressions using muparser"
    LONG_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets
    LIB_DEPENDENCIES libmuparser
    CREDITS "muparser BSD-2-Clause"
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/include)
    target_link_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/lib)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE muparser)

