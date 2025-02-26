cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(python VERSION 5.0)

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

if (BUILD_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    get_target_property(SRC_TST ${PROJECT_NAME} SOURCES)
    get_target_property(INC_TST ${PROJECT_NAME} INCLUDE_DIRECTORIES)
    get_target_property(LIBS_TST ${PROJECT_NAME} LINK_LIBRARIES)
    get_target_property(CXX_STD_TST ${PROJECT_NAME} CXX_STANDARD)

    set(TARGET_TST ${PROJECT_NAME}_test)
    add_executable(${TARGET_TST} ${SRC_TST} test/test.cpp)
    target_include_directories(${TARGET_TST} PRIVATE ${INC_TST} test src)
    target_link_libraries(${TARGET_TST} PRIVATE ${LIBS_TST} Qt6::Test libalbert)
    set_target_properties(${TARGET_TST}
        PROPERTIES
            CXX_STANDARD ${CXX_STD_TST}
            AUTOMOC ON
            AUTOUIC ON
            AUTORCC ON
    )
    set_property(TARGET ${TARGET_TST}
        APPEND PROPERTY AUTOMOC_MACRO_NAMES "ALBERT_PLUGIN")
    add_test(NAME ${TARGET_TST} COMMAND ${TARGET_TST})

endif()


## – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – – –

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
