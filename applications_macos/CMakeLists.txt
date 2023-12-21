if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    project(applications_macos VERSION 5.0)

    albert_plugin(
        SOURCE_FILES
            src/plugin.h
            src/plugin.mm
        PRIVATE_LINK_LIBRARIES
            objc
            "-framework Cocoa"
        TS_FILES
            i18n/${PROJECT_NAME}_de.ts
    )

endif()


