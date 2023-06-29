project(qmlboxmodel VERSION 2.0)
albert_plugin(
    NAME "QML Box Model"
    DESCRIPTION "The QML frontend"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets Qml Quick
    FRONTEND
)

#install(DIRECTORY "styles/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/styles")


