project(files VERSION 1.6)
albert_plugin(
    NAME Files
    DESCRIPTION "Open and browse files"
    LONG_DESCRIPTION "The files plugins provides three extensions to access files. The main extension indexes the file system as specified in the settings and provides a lookup by filename. The other two file browsing extensions are triggered by `/` and `~` and provide a way to browse through the file system using paths. This is handy to access files that are _not_ indexed. In combination with the tab completion this is a nice way to browse the file system."
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets Concurrent
)

option(BUILD_FILES_TESTS "Build ${PROJECT_NAME} tests (Reqiures doctest)" OFF)
if (BUILD_FILES_TESTS)
    find_package(doctest)
    set(TARGET_TST ${PROJECT_NAME}_test)
    add_executable(${TARGET_TST} #${SRC}
        test/test.cpp
        src/fsindexpath.h
        src/fsindexpath.cpp
        src/fsindexnodes.h
        src/fsindexnodes.cpp
        src/fileitems.h
        src/fileitems.cpp
    )

    target_include_directories(${TARGET_TST} PRIVATE
        ${CMAKE_BINARY_DIR}/include
        ../../include
    )

    target_link_libraries(${TARGET_TST} PRIVATE
        doctest::doctest
        Qt6::Core
        albert::albert
    )
endif()
