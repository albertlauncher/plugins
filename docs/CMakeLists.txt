if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
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

find_package(LibArchive REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE LibArchive::LibArchive)
endif()
