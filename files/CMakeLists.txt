cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(files VERSION 8.2)

albert_plugin(
    INCLUDE PRIVATE $<TARGET_PROPERTY:albert::applications,INTERFACE_INCLUDE_DIRECTORIES>
    QT Concurrent Widgets
)

if (BUILD_TESTS)
    set(TARGET_TST ${PROJECT_NAME}_test)
    find_package(doctest)

    add_executable(${TARGET_TST} #${SRC}
        test/test.cpp
        src/fsindex.h
        src/fsindex.cpp
        src/fsindexpath.h
        src/fsindexpath.cpp
        src/fsindexnodes.h
        src/fsindexnodes.cpp
        src/fileitems.h
        src/fileitems.cpp
    )

    set_target_properties(${TARGET_TST} PROPERTIES
        CXX_STANDARD 20
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )

    target_include_directories(${TARGET_TST} PRIVATE src)

    target_link_libraries(${TARGET_TST} PRIVATE
        Qt6::Concurrent
        Qt6::Gui
        albert::libalbert
        doctest::doctest
    )

endif()
