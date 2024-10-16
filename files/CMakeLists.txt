cmake_minimum_required(VERSION 3.16)
find_package(Albert REQUIRED)

project(files VERSION 9.2)

albert_plugin(
    INCLUDE PRIVATE $<TARGET_PROPERTY:albert::applications,INTERFACE_INCLUDE_DIRECTORIES>
    QT Concurrent Widgets
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
    target_link_libraries(${TARGET_TST} PRIVATE ${LIBS_TST} Qt6::Test)
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
