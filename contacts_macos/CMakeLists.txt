if(APPLE)
project(contacts_macos VERSION 1.1)
find_library(CONTACTS_FRAMEWORK Contacts)
albert_plugin(
    NAME Contacts
    DESCRIPTION "Access your contacts"
    LONG_DESCRIPTION "Apple contacts prototype. For now only phone and mail."
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets
    LIB_DEPENDENCIES [Contacts]
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CONTACTS_FRAMEWORK})
endif()
