if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    project(sparkle_updater VERSION 2.1)

    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
    find_package(Sparkle REQUIRED)

    albert_plugin(
        SOURCE_FILES
            src/configwidget.ui
            src/plugin.mm
            src/plugin.h
            cmake/FindSparkle.cmake
        PRIVATE_INCLUDE_DIRECTORIES
            ${SPARKLE_INCLUDE_DIRS}
        PRIVATE_LINK_LIBRARIES
            ${SPARKLE_LIBRARIES}
    )

endif()
