if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    project(platform VERSION 1.2)
    albert_plugin(
        NAME "PlatformServices"
        DESCRIPTION "Access platform services"
        LONG_DESCRIPTION "Provides handlers for platform services. For now only the dictionary."
        LICENSE MIT
        URL https://github.com/albertlauncher/plugins/tree/master/platform_macos
        MAINTAINERS @manuelschneid3r
        QT_DEPENDENCIES Widgets
        LIB_DEPENDENCIES [objc cococa corefoundation]
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa" "-framework CoreFoundation" ${CORE_FOUNDATION_LIBRARY})
endif()
