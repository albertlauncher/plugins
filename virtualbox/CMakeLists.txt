cmake_minimum_required(VERSION 3.1.3)

project(virtualbox)

file(GLOB_RECURSE SRC src/* metadata.json)

find_package(Qt5 5.5.0 REQUIRED COMPONENTS Widgets Xml)

add_library(${PROJECT_NAME} SHARED ${SRC} ${PROJECT_NAME}.qrc)

add_definitions(-DRT_OS_LINUX)
add_definitions(-DVBOX_WITH_XPCOM_NAMESPACE_CLEANUP)
set(PATH_XPCOM /usr/${CMAKE_INSTALL_LIBDIR}/virtualbox/sdk/bindings/xpcom)
set(PATH_BIN /usr/${CMAKE_INSTALL_LIBDIR}/virtualbox)

if(NOT EXISTS ${PATH_XPCOM}/include/VirtualBox_XPCOM.h)
    message("${PATH_XPCOM}/include/VirtualBox_XPCOM.h not found. Build will fail!")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "${PATH_BIN}")

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src/
        ${PATH_XPCOM}/include
        ${PATH_XPCOM}/include/nsprpub
        ${PATH_XPCOM}/include/string
        ${PATH_XPCOM}/include/xpcom
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Widgets
        Qt5::Xml
        albert::lib
        ${PATH_BIN}/VBoxXPCOM.so
        xdg
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert/plugins)
