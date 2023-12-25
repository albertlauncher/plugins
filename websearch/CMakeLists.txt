project(websearch VERSION 8.1)

albert_plugin(
    SOURCE_FILES
        ${PROJECT_NAME}.qrc
        src/*.cpp
        src/*.h
        src/*.ui
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)
