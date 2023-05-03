project(ssh VERSION 1.6)
albert_plugin(
    NAME SecureShell
    DESCRIPTION "Run secure shell sessions"
    LONG_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    EXEC_DEPENDENCIES ssh
    QT_DEPENDENCIES Widgets
)
