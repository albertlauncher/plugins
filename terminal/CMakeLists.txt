project(terminal VERSION 1.3)
albert_plugin(
    NAME Terminal
    DESCRIPTION "Run executables from PATH"
    LICENSE BSD-2
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets Concurrent
)
