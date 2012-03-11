set(SRCS
            src/disasm.cpp
            src/gekko_regs.cpp
            src/main.cpp)

qt4_wrap_ui(UI_HDRS
                    src/disasm.ui
                    src/gekko_regs.ui
                    src/main.ui)

qt4_wrap_cpp(MOC_SRCS
                        src/disasm.hxx
                        src/gekko_regs.hxx
                        src/main.hxx)

# add uic results to include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(gekko-qt ${SRCS} ${MOC_SRCS} ${UI_HDRS})
target_link_libraries(gekko-qt core input_common common ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL_LIBRARY} rt GLEW)

#install(TARGETS gekko-qt RUNTIME DESTINATION ${bindir})
