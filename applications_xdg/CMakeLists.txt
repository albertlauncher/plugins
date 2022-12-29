if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
project(applications VERSION 1.3)
albert_plugin(
    NAME Applications
    DESCRIPTION "Launch applications"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/applications_xdg
    MAINTAINERS @manuelschneid3r
    AUTHORS @manuelschneid3r
    QT_DEPENDENCIES Widgets
)
endif()
