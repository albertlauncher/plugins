if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
project(applications VERSION 1.6)
albert_plugin(
    NAME Applications
    DESCRIPTION "Launch applications"
    LONG_DESCRIPTION "Run desktop applications as described in [freedesktop.org Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/)."
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/applications_xdg
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets
)
endif()
