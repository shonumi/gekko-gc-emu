set(SRCS	src/input_common.cpp
			src/keyboard_input/keyboard_input.cpp
			src/sdl_joypads/sdl_joypads.cpp)

add_library(input_common STATIC ${SRCS})
