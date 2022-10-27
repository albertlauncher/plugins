albert_plugin(
    VERSION 1.2
    NAME "Qt Widgets BoxModel"
    DESCRIPTION "The native frontend"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/widgetsboxmodel
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Svg Widgets StateMachine
    FRONTEND
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${PROJECT_NAME} PRIVATE objc "-framework Cocoa")
endif()

install(DIRECTORY "share/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}")
