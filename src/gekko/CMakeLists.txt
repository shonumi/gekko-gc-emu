set(SRCS	src/gekko.cpp
            src/emuwindow/emuwindow_sdl.cpp)

add_executable(gekko ${SRCS})
target_link_libraries(gekko core video_core input_common common ${OPENGL_LIBRARIES} ${SDL_LIBRARY} rt GLEW)

#install(TARGETS gekko RUNTIME DESTINATION ${bindir})
