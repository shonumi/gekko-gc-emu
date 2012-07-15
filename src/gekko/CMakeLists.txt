set(SRCS	src/gekko.cpp
            src/emuwindow/emuwindow_sdl.cpp
            src/emuwindow/emuwindow_glfw.cpp)

# NOTE: This is a workaround for CMake bug 0006976 (missing X11_xf86vmode_LIB variable)
if (NOT X11_xf86vmode_LIB)
    set(X11_xv86vmode_LIB Xxf86vm)
endif()

add_executable(gekko ${SRCS})
target_link_libraries(gekko core video_core input_common common ${OPENGL_LIBRARIES} ${SDL2_LIBRARY} ${GLFW_LIBRARIES} GLEW rt ${X11_Xrandr_LIB} ${X11_xv86vmode_LIB})

#install(TARGETS gekko RUNTIME DESTINATION ${bindir})
