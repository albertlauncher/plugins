cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(python VERSION 4.12)

set(PYBIND11_FINDPYTHON ON)
#find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(pybind11)

albert_plugin(
    INCLUDE PRIVATE $<TARGET_PROPERTY:albert::applications,INTERFACE_INCLUDE_DIRECTORIES>
    LINK PRIVATE pybind11::embed
    QT Concurrent Widgets
)

target_sources(${PROJECT_NAME} PRIVATE albert.pyi)

install(
    DIRECTORY "plugins/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/albert/${PROJECT_NAME}/plugins"
    REGEX "plugins\\/\\..+" EXCLUDE  # exclude hidden files
    PATTERN "__pycache__" EXCLUDE
    PATTERN "*.md" EXCLUDE
)





## – –







# find_package(Python COMPONENTS Interpreter Development REQUIRED)

# message(STATUS "Python3_INCLUDE_DIRS ${Python3_INCLUDE_DIRS}")
# message(STATUS "Python3_INTERPRETER ${Python3_INTERPRETER}")
# message(STATUS "Python3_LIBRARIES ${Python3_LIBRARIES}")
# message(STATUS "Python3_LIBRARY ${Python3_LIBRARY}")
# message(STATUS "Python_EXECUTABLE ${Python_EXECUTABLE}")
# message(STATUS "Python_INCLUDE_DIRS ${Python_INCLUDE_DIRS}")
# message(STATUS "Python_INTERPRETER ${Python_INTERPRETER}")
# message(STATUS "Python_LIBRARIES ${Python_LIBRARIES}")
# message(STATUS "Python_LIBRARY ${Python_LIBRARY}")


# # set(PYTHON_LIBRARY ${Python_LIBRARIES})
# message(STATUS "PYTHON_LIBRARY ${PYTHON_LIBRARY}")

# set(Python_LIBRARY ${Python_LIBRARIES})
# message(STATUS "Python_LIBRARY ${Python_LIBRARY}")
# message(STATUS "PYTHON_LIBRARY ${PYTHON_LIBRARY}")

# if (NOT Python_FOUND)
#     message(FATAL_ERROR "Python3 not found")
# else()
# endif()







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
