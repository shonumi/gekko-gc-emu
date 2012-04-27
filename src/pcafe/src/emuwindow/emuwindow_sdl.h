#ifndef __EMUWINDOW_SDL_H__
#define __EMUWINDOW_SDL_H__

#include "SDL.h"
#include "video/emuwindow.h"


#if SDL_MAJOR_VERSION == 1

class EmuWindow_SDL : public EmuWindow
{
public:
    EmuWindow_SDL();
    ~EmuWindow_SDL();

    void SwapBuffers();
    void SetTitle(const char* title);
};

#elif SDL_MAJOR_VERSION == 2

class SDL_Window;
class EmuWindow_SDL : public EmuWindow
{
public:
    EmuWindow_SDL();
    ~EmuWindow_SDL();

    void SwapBuffers();
    void SetTitle(const char* title);

    SDL_Window *mainwindow;
};

#else
#error Unsupported SDL version?
#endif

#endif // __EMUWINDOW_SDL_H__
