project(widgetsboxmodel VERSION 7.0)

find_package(Qt6 REQUIRED COMPONENTS Svg StateMachine)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/*.cpp
        src/*.h
        src/*.ui
    PRIVATE_LINK_LIBRARIES
        Qt6::StateMachine
        Qt6::Svg
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)

install(DIRECTORY "themes/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/themes")
