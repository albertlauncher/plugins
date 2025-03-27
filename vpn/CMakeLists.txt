cmake_minimum_required(VERSION 3.16)

find_package(Albert REQUIRED)

project(vpn VERSION 6.0)

file(GLOB_RECURSE I18N_SOURCES src/*)

albert_plugin(
    SOURCES
        src/plugin.h
        src/vpnconnectionitem.h
        src/vpnconnectionitem.cpp
        vpn.qrc
    INCLUDE
        PRIVATE
            src
    I18N_SOURCES
        ${I18N_SOURCES}
    QT
        Widgets
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework SystemConfiguration")
    #target_compile_options(${PROJECT_NAME} PRIVATE "-fobjc-arc")
    target_include_directories(${PROJECT_NAME} PRIVATE src/mac)
    target_sources(${PROJECT_NAME} PRIVATE
        src/mac/plugin.cpp
        src/mac/item.mm
        src/mac/item.h
    )
elseif(UNIX)
    find_package(Qt6 6.2 REQUIRED COMPONENTS DBus)
    target_link_libraries(${PROJECT_NAME} PRIVATE "Qt6::DBus")

    # DBus interface
    set_source_files_properties(src/xdg/NetworkManager.xml PROPERTIES NO_NAMESPACE ON)
    set_source_files_properties(src/xdg/NetworkManager.xml PROPERTIES INCLUDE custom_types.h)
    qt_add_dbus_interface(DBUS_SRCS src/xdg/NetworkManager.xml nm)

    target_include_directories(${PROJECT_NAME}
        PRIVATE
            src/xdg  # for custom_types.h
            ${PROJECT_BINARY_DIR}  # for the generated files
    )
    target_sources(${PROJECT_NAME} PRIVATE
        src/xdg/plugin.cpp
        src/xdg/networkmanager.cpp
        src/xdg/networkmanager.h
        src/xdg/item.cpp
        src/xdg/item.h
        src/xdg/custom_types.h
        src/xdg/NetworkManager.xml
        ${PROJECT_BINARY_DIR}/nm.h
        ${DBUS_SRCS}
    )

endif()
