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

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_program(BREW_EXECUTABLE brew)
    execute_process(
        COMMAND "${BREW_EXECUTABLE}" --prefix
        OUTPUT_VARIABLE BREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    target_include_directories(${PROJECT_NAME} PRIVATE "${BREW_PREFIX}/include")
    target_link_directories(${PROJECT_NAME} PRIVATE "${BREW_PREFIX}/lib")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE qalculate)

