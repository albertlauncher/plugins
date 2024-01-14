project(calculator_qalculate VERSION 5.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(PkgConfig)
pkg_check_modules(LIBQALCULATE REQUIRED libqalculate)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/configwidget.ui
        src/plugin.cpp
        src/plugin.h
    PRIVATE_INCLUDE_DIRECTORIES
        ${LIBQALCULATE_INCLUDE_DIRS}
    PRIVATE_LINK_LIBRARIES
        ${LIBQALCULATE_LIBRARIES}
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)

target_link_directories(${PROJECT_NAME} PRIVATE ${LIBQALCULATE_LIBRARY_DIRS})
