project(docs VERSION 1.4)
albert_plugin(
    NAME Docs
    DESCRIPTION "Documentation quick access"
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets Sql Network
    LIB_DEPENDENCIES libarchive
    CREDITS "libarchive BSD-3-clause, Zeal/Dash for docfiles and endpoints"
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Homebrew ships libarchive keg only, include dirs have to be set manually
    execute_process(
        COMMAND brew --prefix libarchive
        OUTPUT_VARIABLE LIBARCHIVE_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    set(LibArchive_INCLUDE_DIR "${LIBARCHIVE_PREFIX}/include")
endif()
find_package(LibArchive REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE LibArchive::LibArchive)


