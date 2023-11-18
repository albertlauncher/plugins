project(python VERSION 2.1.0)
albert_plugin(
    NAME "Python"
    DESCRIPTION "Load Python plugins"
    LONG_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    LICENSE ALL
    URL https://github.com/albertlauncher/plugins/tree/master/${PROJECT_NAME}
    MAINTAINERS @manuelschneid3r
    QT_DEPENDENCIES Widgets
    CREDITS "pybind11 BSD-3-Clause"
    NOUNLOAD
)

find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED)

configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/src/config.h"
    @ONLY
)
target_sources(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}/src/config.h")
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}/src")
target_link_libraries(${PROJECT_NAME} PRIVATE pybind11::embed)

install(
    DIRECTORY "plugins/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/plugins"
    REGEX "/\\..*" EXCLUDE  # exclude hidden files
    PATTERN "__pycache__" EXCLUDE
)

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
