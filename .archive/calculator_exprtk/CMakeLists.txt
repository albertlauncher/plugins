project(calculator_exprtk VERSION 1.0)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig)
pkg_check_modules(MPFR REQUIRED mpfr)
# message(MPFR_INCLUDE_DIRS: ${MPFR_INCLUDE_DIRS})
# message(MPFR_LIBRARIES: ${MPFR_LIBRARIES})
# message(MPFR_LIBRARY_DIRS: ${MPFR_LIBRARY_DIRS})

albert_plugin(
    INCLUDE_DIRECTORIES SYSTEM
        PRIVATE
            ${MPFR_INCLUDE_DIRS}
    LINK_LIBRARIES
        PRIVATE
            ${MPFR_LIBRARIES}
)

target_link_directories(${PROJECT_NAME} PRIVATE ${MPFR_LIBRARY_DIRS})
