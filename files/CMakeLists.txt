project(files VERSION 8.0)

find_package(Qt6 REQUIRED COMPONENTS Concurrent)

albert_plugin(
    SOURCE_FILES
        src/*.cpp
        src/*.h
        src/*.ui
    PRIVATE_LINK_LIBRARIES
        Qt6::Concurrent
    TS_FILES
        i18n/${PROJECT_NAME}_de.ts
)

if (BUILD_TESTS)
    find_package(doctest)
    set(TARGET_TST ${PROJECT_NAME}_test)
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
