if(APPLE)

    cmake_minimum_required(VERSION 3.16)
    find_package(Albert REQUIRED)

    project(bluetooth VERSION 2.0)

    albert_plugin(
        LINK PRIVATE "objc" "-framework IOBluetooth"
        QT Core
    )
endif()
