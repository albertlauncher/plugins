cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(python VERSION 4.3)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

albert_plugin(
    LINK PRIVATE pybind11::embed
    QT Concurrent Widgets
)

target_sources(${PROJECT_NAME} PRIVATE albert.pyi)

install(
    DIRECTORY "plugins/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/plugins"
    REGEX "/\\..*" EXCLUDE  # exclude hidden files
    PATTERN "__pycache__" EXCLUDE
)

## – –

#find_program(
#    Python3_EXECUTABLE python3
#    HINTS /opt/homebrew/bin
#)

#message(STATUS ${Python3_EXECUTABLE})

#option(BUILD_PY_TESTS "Build ${PROJECT_NAME} tests (Reqiures doctest)" OFF)
#if (BUILD_PY_TESTS)
#    set(TARGET_TST ${PROJECT_NAME}_test)
#    find_package(doctest)


#    include(GenerateExportHeader)
#    generate_export_header(${TARGET_LIB}
#        BASE_NAME "ALBERT"
#        EXPORT_FILE_NAME "${PROJECT_BINARY_DIR}/include/albert/export.h"
#    )


#    add_executable(${TARGET_TST}
#        test/test.cpp
#        "${PROJECT_BINARY_DIR}/include/albert/export.h"
#        ../../src/plugininstanceprivate.h
#        ../../src/plugininstanceprivate.cpp
#        ../../src/pluginloaderprivate.h
#        ../../include/albert/extension/pluginprovider/pluginloader.h
#        ../../src/pluginloader.cpp
#    )

#    target_include_directories(${TARGET_TST} PRIVATE
#        ${PROJECT_BINARY_DIR}/include
#        include
#        src
#        ../../include
#        ../../src
#    )

#    target_link_libraries(${TARGET_TST}
#        PRIVATE
#        pybind11::embed
#        doctest::doctestQt6::Core
##        albert::albert
#    )
#endif()
