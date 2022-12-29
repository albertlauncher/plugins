project(ssh VERSION 1.3)
albert_plugin(
    NAME SecureShell
    DESCRIPTION "Run secure shell sessions"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    EXEC_DEPENDENCIES ssh
    QT_DEPENDENCIES Widgets
)
