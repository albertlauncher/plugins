if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    project(vpn VERSION 1.0)
    albert_plugin(
        NAME "VPN"
        DESCRIPTION "Control VPN connections"
        LICENSE MIT
        URL https://github.com/albertlauncher/plugins/tree/master/vpn
        MAINTAINERS @manuelschneid3r
        QT_DEPENDENCIES Widgets
        LIB_DEPENDENCIES SystemConfiguration
    )
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            "-framework SystemConfiguration"
        )
endif()
