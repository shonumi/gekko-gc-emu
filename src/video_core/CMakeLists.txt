set(SRCS
            src/bp_mem.cpp
            src/cp_mem.cpp
            src/xf_mem.cpp
            src/fifo.cpp
            src/vertex_loader.cpp
            src/video_core.cpp
            src/renderer_gl3/renderer_gl3.cpp
            src/renderer_gl3/shader_manager.cpp)

add_library(video_core STATIC ${SRCS})
