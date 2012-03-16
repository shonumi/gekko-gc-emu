#ifndef __EMUWINDOW_H__
#define __EMUWINDOW_H__

// Abstraction class used to provide an interface between emulation code and the frontend (e.g. SDL_Window, QGLWidget, GLFW, etc...)
class EmuWindow
{
public:
    virtual void SwapBuffers() = 0;
    virtual void SetTitle(const char* title) = 0;

protected:
    EmuWindow() {}
    virtual ~EmuWindow() {}
};

#endif // __EMUWINDOW_H__
