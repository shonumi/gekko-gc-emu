#include "SDL.h"
#include "emuwindow_sdl.h"


// SDL 1 implementation
#if SDL_MAJOR_VERSION == 1

EmuWindow_SDL::EmuWindow_SDL()
{
    const SDL_VideoInfo* video;

    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    /* Quit SDL properly on exit */
    atexit(SDL_Quit);

    /* Get the current video information */
    video = SDL_GetVideoInfo( );
    if( video == NULL ) {
        fprintf(stderr, "Couldn't get video information: %s\n", SDL_GetError());
        exit(1);
    }

    /* Set the minimum requirements for the OpenGL window */
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    if( SDL_SetVideoMode( 640, 480, video->vfmt->BitsPerPixel, SDL_OPENGL ) == 0 ) {
        fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_WM_SetCaption(/*g_window_title*//*TODO*/"", NULL);
}
EmuWindow_SDL::~EmuWindow_SDL()
{
    // TODO..
}

void EmuWindow_SDL::SwapBuffers()
{
    SDL_GL_SwapBuffers();
}
void EmuWindow_SDL::SetTitle(const char* title)
{
    SDL_WM_SetCaption(title, NULL);
}

// SDL 2 implementation
#elif SDL_MAJOR_VERSION == 2

EmuWindow_SDL::EmuWindow_SDL()
{
    SDL_GLContext maincontext; /* Our opengl context handle */

    OPENGL_Kill(); // TODO: Doesn't belong here

    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK    ) != 0 ) {
        printf("Unable to initialize SDL: %s\n", SDL_GetError());
        return;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    mainwindow = SDL_CreateWindow("gekko", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    /* Create our opengl context and attach it to our window */
    maincontext = SDL_GL_CreateContext(mainwindow);

    /* This makes our buffer swap syncronized with the monitor's vertical refresh */
    SDL_GL_SetSwapInterval(1);
}

EmuWindow_SDL::~EmuWindow_SDL()
{

}

void EmuWindow_SDL::SwapBuffers()
{
    SDL_GL_SwapWindow(mainwindow);
}

void EmuWindow_SDL::SetTitle(const char* title)
{
    SDL_SetWindowTitle(mainwindow, title);
}

#endif
