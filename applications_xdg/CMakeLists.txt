if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
albert_plugin(
    VERSION 1.3
    ID applications
    NAME Applications
    DESCRIPTION "Launch applications"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/applications_xdg
    MAINTAINERS @manuelschneid3r
    AUTHORS @manuelschneid3r
    QT_DEPENDENCIES Widgets
)
endif()
