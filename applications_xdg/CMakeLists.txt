if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
project(applicationss VERSION 1.4)
albert_plugin(
    NAME Applications
    DESCRIPTION "Launch applications"
    LONG_DESCRIPTION "Run desktop applications as described in [freedesktop.org Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/)."
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/applications_xdg
    MAINTAINERS @manuelschneid3r
    AUTHORS @manuelschneid3r
    QT_DEPENDENCIES Widgets
)
endif()
