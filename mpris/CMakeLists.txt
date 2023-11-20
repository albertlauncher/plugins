project(mpris VERSION 1.2)
albert_plugin(
    NAME "MPRIS Control"
    DESCRIPTION "Control MPRIS media players. If you have MPRIS-capable media players running (like Rhythmbox, VLC, ...) just type any of 'play', 'pause', 'stop', 'next', 'previous' and you can send these commands directly to your player from albert."
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @idkcpp @manuelschneid3r @luksfarris
    QT_DEPENDENCIES Widgets DBus
)

