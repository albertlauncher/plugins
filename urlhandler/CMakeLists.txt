project(urlhandler VERSION 1.3)
albert_plugin(
    NAME "URL Handler"
    DESCRIPTION "Open URLs"
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Gui  # DesktopServices
)
