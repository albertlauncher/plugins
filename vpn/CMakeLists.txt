if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    project(vpn VERSION 2.0)

    albert_plugin(
        SOURCE_FILES
            src/plugin.mm
            src/plugin.h
        PRIVATE_LINK_LIBRARIES
            "-framework SystemConfiguration"
        TS_FILES
            i18n/${PROJECT_NAME}_de.ts
    )

endif()
