project(widgetsboxmodel VERSION 7.1)

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
)

install(DIRECTORY "themes/" DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/themes")
