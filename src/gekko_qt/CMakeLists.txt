set(SRCS
            src/bootmanager.cpp
            src/callstack.cpp
            src/disasm.cpp
            src/gamelist.cpp
            src/gekko_regs.cpp
            src/hotkeys.cpp
            src/image_info.cpp
            src/main.cpp
            src/path_list.cpp
            src/ramview.cpp
            src/welcome_wizard.cpp
            src/config/controller_config.cpp
            src/config/controller_config_util.cpp)

qt4_wrap_ui(UI_HDRS
                    src/callstack.ui
                    src/disasm.ui
                    src/gekko_regs.ui
                    src/hotkeys.ui
                    src/image_info.ui
                    src/main.ui
                    src/welcome_wizard.ui
                    src/config/controller_config.ui)

qt4_wrap_cpp(MOC_SRCS
                        src/bootmanager.hxx
                        src/callstack.hxx
                        src/disasm.hxx
                        src/gamelist.hxx
                        src/gekko_regs.hxx
                        src/hotkeys.hxx
                        src/image_info.hxx
                        src/main.hxx
                        src/path_list.hxx
                        src/ramview.hxx
                        src/welcome_wizard.hxx
                        src/config/controller_config.hxx
                        src/config/controller_config_util.hxx)

# add uic results to include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(./src/)

add_executable(gekko-qt ${SRCS} ${MOC_SRCS} ${UI_HDRS})
target_link_libraries(gekko-qt core video_core input_common common qhexedit ${QT_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARY} rt GLEW ${GLFW_LIBRARIES})

#install(TARGETS gekko-qt RUNTIME DESTINATION ${bindir})
