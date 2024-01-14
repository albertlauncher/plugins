if(APPLE)

project(contacts_macos VERSION 4.0)

find_library(CONTACTS_FRAMEWORK Contacts)

albert_plugin(
    SOURCE_FILES
        src/plugin.h
        src/plugin.mm
    PRIVATE_LINK_LIBRARIES
        ${CONTACTS_FRAMEWORK}
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)

endif()
