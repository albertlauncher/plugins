if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    project(sparkle_updater VERSION 1.0)
    albert_plugin(
        NAME Updater
        DESCRIPTION "Keep the app updated"
        LICENSE ALL
        URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
        MAINTAINERS @manuelschneid3r
        QT_DEPENDENCIES Widgets
        NOUNLOAD
    )

    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    find_package(Sparkle REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SPARKLE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SPARKLE_LIBRARIES})

endif()
