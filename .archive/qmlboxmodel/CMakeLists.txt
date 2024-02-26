project(qmlboxmodel VERSION 3.0)

find_package(Qt6 REQUIRED COMPONENTS Qml Quick)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        resources/qml/*
        src/configwidget.ui
        src/imageprovider*
        src/plugin*
        src/qmlinterface*
        src/window*
    PRIVATE_LINK_LIBRARIES
        Qt6::Qml
        Qt6::Quick
)

target_sources(qmlboxmodel
  PRIVATE
    resources/qml/StateChart.scxml
)

#install(DIRECTORY "styles/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/styles")


