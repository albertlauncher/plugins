cmake_minimum_required(VERSION 3.16)

if(APPLE)

    find_package(Albert REQUIRED)

    project(vpn VERSION 4.0)

    albert_plugin(
        LINK PRIVATE "-framework SystemConfiguration"
        QT Widgets
    )

endif()
