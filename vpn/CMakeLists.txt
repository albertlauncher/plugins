if(APPLE)

    cmake_minimum_required(VERSION 3.16)
    find_package(Albert REQUIRED)

    project(vpn VERSION 2.2)

    albert_plugin(
        LINK PRIVATE "-framework SystemConfiguration"
        QT Widgets
    )

endif()
