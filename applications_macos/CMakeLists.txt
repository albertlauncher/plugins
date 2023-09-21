if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    project(applications_macos VERSION 1.3)
    albert_plugin(
        NAME Applications
        DESCRIPTION "Launch applications"
        LICENSE ALL
        URL https://github.com/albertlauncher/plugins/tree/master/applications_macos
        MAINTAINERS @manuelschneid3r
        QT_DEPENDENCIES Widgets
        LIB_DEPENDENCIES [objc cococa]
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
endif()


