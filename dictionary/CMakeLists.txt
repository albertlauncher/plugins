if(APPLE)

    cmake_minimum_required(VERSION 3.16)
    find_package(Albert REQUIRED)

    project(dictionary VERSION 4.1)

    find_library(CORESERVICES_LIBRARY CoreServices)

    albert_plugin(
        LINK PRIVATE ${CORESERVICES_LIBRARY}
        QT Core
    )

endif()
