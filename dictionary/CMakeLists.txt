if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    project(dictionary VERSION 3.0)

    albert_plugin(
        SOURCE_FILES
            ${PROJECT_NAME}.qrc
            src/configwidget.ui
            src/plugin.h
            src/plugin.mm
        PRIVATE_LINK_LIBRARIES
            objc
            "-framework Cocoa"
            "-framework CoreFoundation"
            ${CORE_FOUNDATION_LIBRARY}
        TS_FILES
            i18n/${PROJECT_NAME}_de.ts
    )

endif()
