set(SRCS	src/input_common.cpp
			src/sdl_keys/sdl_keys.cpp
			src/sdl_joypads/sdl_joypads.cpp)

add_library(input_common STATIC ${SRCS})
