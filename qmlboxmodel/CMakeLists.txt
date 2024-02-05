project(qmlboxmodel VERSION 3.0)

find_package(Qt6 REQUIRED COMPONENTS Qml Quick)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        resources/qml/*
        src/*.cpp
        src/*.h
        src/configwidget.ui
    PRIVATE_LINK_LIBRARIES
        Qt6::Qml
        Qt6::Quick
)

#install(DIRECTORY "styles/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/styles")


