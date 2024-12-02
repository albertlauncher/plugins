if(APPLE)

cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(contacts VERSION 5.2)

find_library(CONTACTS_FRAMEWORK Contacts)

albert_plugin(
    LINK PRIVATE ${CONTACTS_FRAMEWORK}
    QT Widgets Concurrent
)

endif()
