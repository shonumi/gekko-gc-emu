cmake_minimum_required(VERSION 2.6)

project(gekko)

# silence some spam
add_definitions(-Wno-attributes)

# dependency checking
include(FindSDL REQUIRED)
include(FindOpenGL REQUIRED)

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
