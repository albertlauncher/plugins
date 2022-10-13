find_package(albert REQUIRED)

albert_plugin(
        ID template
        VERSION 0.0
        NAME Template
        DESCRIPTION "For development purposes"
        LICENSE BSD-2
        URL https://github.com/albertlauncher/plugins/template
        MAINTAINERS @ManuelSchneid3r
        QT_DEPENDENCIES Widgets
        LIB_DEPENDENCIES
        EXEC_DEPENDENCIES
)
