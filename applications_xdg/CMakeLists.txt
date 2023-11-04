if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
project(applications_xdg VERSION 1.8)
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
