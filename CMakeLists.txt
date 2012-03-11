cmake_minimum_required(VERSION 2.6)

project(gekko)

# silence some spam
add_definitions(-Wno-attributes)
add_definitions(-DSINGLETHREADED)

# dependency checking
include(FindSDL REQUIRED)
include(FindOpenGL REQUIRED)

option(DISABLE_QT4 "Disable Qt4 GUI" OFF)
if(NOT DISABLE_QT4)
    include(FindQt4)
    find_package(Qt4 COMPONENTS QtCore QtGui)

    if(QT4_FOUND AND QT_QTCORE_FOUND AND QT_QTGUI_FOUND)
        include(${QT_USE_FILE})
        include_directories(${QT_INCLUDES})
    else()
        message("Qt4 libraries not found! Disabling Qt4 GUI")
    endif()
endif()

# external includes
include_directories(${SDL_INCLUDE_DIR})
include_directories(externals/rapidxml-1.13)
include_directories(${OPENGL_INCLUDE_DIR})

# internal includes
include_directories(src/common/src)
include_directories(src/core/src)
include_directories(src/input_common/src)
include_directories(src/win32/src)

# process subdirectories
add_subdirectory(src)
