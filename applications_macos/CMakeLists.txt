if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
albert_plugin(
        VERSION 1.0
        ID applications
        NAME Applications
        DESCRIPTION "Launch applications"
        LICENSE ALL
        URL https://github.com/albertlauncher/plugins/tree/master/applications_macos
        MAINTAINERS @manuelschneid3r
        AUTHORS @manuelschneid3r
        QT_DEPENDENCIES Widgets
        LIB_DEPENDENCIES [objc cococa]
)
target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
endif()


