if(NOT APPLE)

    cmake_minimum_required(VERSION 3.17)
    find_package(Albert REQUIRED)

    project(docs VERSION 6.5)

    find_package(LibArchive REQUIRED)

    albert_plugin(
        LINK PRIVATE LibArchive::LibArchive
        QT Network Sql Widgets
    )

endif()
