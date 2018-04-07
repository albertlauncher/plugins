# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Set the RPATH for the library lookup
set(CMAKE_INSTALL_RPATH "$ORIGIN/../")

# FrontendPlugins

option(BUILD_WIDGETBOXMODEL "Build the frontend" ON)
if (BUILD_WIDGETBOXMODEL)
    add_subdirectory(widgetboxmodel)
endif()

option(BUILD_QMLBOXMODEL "Build the frontend" ON)
if (BUILD_QMLBOXMODEL)
    add_subdirectory(qmlboxmodel)
endif()



# ExtensionPlugins

option(BUILD_APPLICATIONS "Build the extension" ON)
if (BUILD_APPLICATIONS)
    add_subdirectory(applications)
endif()

option(BUILD_CALCULATOR "Build the extension" ON)
if (BUILD_CALCULATOR)
    add_subdirectory(calculator)
endif()

option(BUILD_CHROMEBOOKMARKS "Build the extension" ON)
if (BUILD_CHROMEBOOKMARKS)
    add_subdirectory(chromebookmarks)
endif()

option(BUILD_EXTERNALEXTENSIONS "Build the extension" ON)
if (BUILD_EXTERNALEXTENSIONS)
    add_subdirectory(externalextensions)
endif()

option(BUILD_DEBUG "Build the extension" OFF)
if (BUILD_DEBUG)
    add_subdirectory(debug)
endif()

option(BUILD_FILES "Build the extension" ON)  # Deprecated
if (BUILD_FILES)
    add_subdirectory(files)
endif()

option(BUILD_FIREFOXBOOKMARKS "Build the extension" ON)
if (BUILD_FIREFOXBOOKMARKS)
    add_subdirectory(firefoxbookmarks)
endif()

option(BUILD_HASHGENERATOR "Build the extension" ON)
if (BUILD_HASHGENERATOR)
    add_subdirectory(hashgenerator)
endif()

option(BUILD_MPRIS "Build the extension" ON)
if (BUILD_MPRIS)
    add_subdirectory(mpris)
endif()

option(BUILD_PYTHON "Build the extension" ON)
if (BUILD_PYTHON)
    add_subdirectory(python)
endif()

option(BUILD_QALCULATOR "Build the extension" ON)
if (BUILD_QALCULATOR)
    add_subdirectory(qalculator)
endif()

option(BUILD_SNIPPETS "Build the extension" ON)
if (BUILD_SNIPPETS)
    add_subdirectory(snippets)
endif()

option(BUILD_SSH "Build the extension" ON)
if (BUILD_SSH)
    add_subdirectory(ssh)
endif()

option(BUILD_SYSTEM "Build the extension" ON)
if (BUILD_SYSTEM)
    add_subdirectory(system)
endif()

option(BUILD_TEMPLATE "Build the extension" OFF)
if (BUILD_TEMPLATE)
    add_subdirectory(templateExtension)
endif()

option(BUILD_TERMINAL "Build the extension" ON)
if (BUILD_TERMINAL)
    add_subdirectory(terminal)
endif()

option(BUILD_VIRTUALBOX "Build the extension" OFF)  # Deprecated
if (BUILD_VIRTUALBOX)
    add_subdirectory(virtualbox)
endif()

option(BUILD_WEBSEARCH "Build the extension" ON)
if (BUILD_WEBSEARCH)
    add_subdirectory(websearch)
endif()
