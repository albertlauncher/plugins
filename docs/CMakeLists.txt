cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(docs VERSION 8.0)

# Homebrew ships libarchive keg only, include dirs have to be set manually
# Thats fragile crap but we are not allowed to ship it on macOS anyway.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LibArchive_INCLUDE_DIR "/opt/homebrew/opt/libarchive/include")
endif()


find_package(LibArchive REQUIRED)

albert_plugin(
    LINK PRIVATE LibArchive::LibArchive
    QT Network Sql Widgets
)
