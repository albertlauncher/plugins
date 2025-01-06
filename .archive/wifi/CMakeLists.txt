if(APPLE)

    cmake_minimum_required(VERSION 3.16)
    find_package(Albert REQUIRED)

    project(wifi VERSION 1.0)

    albert_plugin(
        LINK PRIVATE
            "objc"
            "-framework CoreLocation"  # For ssid retrieval
            "-framework CoreWLAN"
            "-framework Security"  # for keychain access
            "-framework Foundation"
        QT Core
    )

    target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")

endif()
