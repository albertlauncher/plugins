#if(NOT APPLE)

    cmake_minimum_required(VERSION 3.16)
    find_package(Albert REQUIRED)

    project(docs VERSION 7.1)

    find_package(LibArchive REQUIRED)

    albert_plugin(
        LINK PRIVATE LibArchive::LibArchive
        QT Network Sql Widgets
    )

#endif()
