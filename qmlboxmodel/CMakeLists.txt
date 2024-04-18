project(qmlboxmodel VERSION 3.0)

find_package(Qt6 REQUIRED COMPONENTS Qml Quick)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        resources/qml/*.qml
        resources/qml/*.js
        src/configwidget.ui
        src/*/*.h
        src/*/*.cpp
        src/*.h
        src/*.cpp
    QT Qml Quick Widgets
)

# target_sources(qmlboxmodel
#   PRIVATE
#     resources/qml/StateChart.scxml
# )

#install(DIRECTORY "styles/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/styles")


target_compile_definitions(${PROJECT_NAME} PRIVATE QT_QML_DEBUG)
