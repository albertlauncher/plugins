if(APPLE)

    cmake_minimum_required(VERSION 3.16)
    find_package(Albert REQUIRED)

    project(bluetooth VERSION 1.4)

    albert_plugin(
        LINK PRIVATE "objc" "-framework IOBluetooth"
        QT Core
    )

    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")

endif()
