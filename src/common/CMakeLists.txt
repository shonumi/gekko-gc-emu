set(SRCS	src/config.cpp
			src/crc.cpp
			src/log.cpp
			src/timer.cpp
			src/xml.cpp)

add_library(common STATIC ${SRCS})
