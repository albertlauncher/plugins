project(docs VERSION 1.0)
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
    target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/opt/libarchive/include)
    target_link_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/opt/libarchive/lib)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE archive)

