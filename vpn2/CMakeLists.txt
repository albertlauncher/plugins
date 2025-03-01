cmake_minimum_required(VERSION 3.16)


find_package(Albert REQUIRED)

project(vpn2 VERSION 4.0)

albert_plugin(
    INCLUDE PRIVATE src
    QT Widgets DBus
)

# no namespaces (class/namespace conflict)
set_source_files_properties(NetworkManager.xml PROPERTIES NO_NAMESPACE ON)
set_source_files_properties(NetworkManager.xml PROPERTIES INCLUDE custom_types.h)

qt_add_dbus_interface(DBUS_SRCS NetworkManager.xml nm)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src  # for custom_types.h
        ${PROJECT_BINARY_DIR}  # for the generated files
)

target_sources(${PROJECT_NAME}
    PRIVATE
        NetworkManager.xml
        ${PROJECT_BINARY_DIR}/nm.h
        ${DBUS_SRCS}
)
