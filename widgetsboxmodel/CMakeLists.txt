project(widgetsboxmodel VERSION 1.5)
albert_plugin(
    NAME "Widgets BoxModel"
    DESCRIPTION "The native frontend"
    LONG_DESCRIPTION "See the [online documentation](https://github.com/albertlauncher/plugins/blob/master/widgetsboxmodel/README.md)."
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Svg Widgets StateMachine
    FRONTEND
)

install(DIRECTORY "themes/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/themes")
