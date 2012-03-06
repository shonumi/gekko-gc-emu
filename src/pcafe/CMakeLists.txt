set(SRCS	src/pcafe.cpp)

add_executable(pcafe ${SRCS})
target_link_libraries(pcafe core input_common common)

#install(TARGETS pcafe RUNTIME DESTINATION ${bindir})
