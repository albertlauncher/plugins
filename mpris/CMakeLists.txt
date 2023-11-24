if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
project(mpris VERSION 2.0)
albert_plugin(
    NAME "MPRIS"
    DESCRIPTION "Control MPRIS capable media players"
    LONG_DESCRIPTION
        "Use 'play', 'pause', 'stop', 'next' or 'previous' to control \
        MPRIS-capable media players (like Rhythmbox, VLC, …)."
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @idkcpp @manuelschneid3r @luksfarris
    QT_DEPENDENCIES Widgets DBus
)
endif()
