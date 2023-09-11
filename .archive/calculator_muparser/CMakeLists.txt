project(calculator_muparser VERSION 1.7)
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

target_link_libraries(${PROJECT_NAME} PRIVATE muparser)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nested-anon-types)
