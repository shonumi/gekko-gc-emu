set(SRCS	src/pcafe.cpp)

add_executable(pcafe ${SRCS})
target_link_libraries(pcafe core video_core input_common common ${OPENGL_LIBRARIES} ${SDL_LIBRARY} rt GLEW ${GLFW_LIBRARIES})

#install(TARGETS pcafe RUNTIME DESTINATION ${bindir})
