if(APPLE)

cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(contacts_macos VERSION 4.1)

find_library(CONTACTS_FRAMEWORK Contacts)

albert_plugin(
    LINK PRIVATE ${CONTACTS_FRAMEWORK}
    QT Widgets
)

target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")

endif()
