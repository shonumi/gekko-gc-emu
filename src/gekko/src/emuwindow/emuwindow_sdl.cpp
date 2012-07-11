#include "common.h"
#include "emuwindow_sdl.h"

EmuWindow_SDL::EmuWindow_SDL() {

    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK    ) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    main_window_ = SDL_CreateWindow("ChronoBarkangel was here", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    /* Create our opengl context and attach it to our window */
    main_context_ = SDL_GL_CreateContext(main_window_);

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);

    DoneCurrent();
}

EmuWindow_SDL::~EmuWindow_SDL() {
}

/// Makes the graphics context current for the caller thread
void EmuWindow_SDL::MakeCurrent() {
    SDL_GL_MakeCurrent(main_window_, main_context_);
}

/// Releases (dunno if this is the "right" word) the GLFW context from the caller thread
void EmuWindow_SDL::DoneCurrent() {
    SDL_GL_MakeCurrent(main_window_, 0);
}

void EmuWindow_SDL::SwapBuffers() {
    SDL_GL_SwapWindow(main_window_);
}

void EmuWindow_SDL::SetTitle(const char* title) {
    SDL_SetWindowTitle(main_window_, title);
}

void EmuWindow_SDL::GetWindowSize(int &width, int &height) {
    width = 640;
    height = 480;
}

void EmuWindow_SDL::SetConfig(Config config) {
}
