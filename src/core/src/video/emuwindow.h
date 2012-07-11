#include "common.h"

#ifndef __EMUWINDOW_H__
#define __EMUWINDOW_H__

// Abstraction class used to provide an interface between emulation code and the frontend (e.g. SDL, 
//  QGLWidget, GLFW, etc...)
class EmuWindow
{

public:
    /// Data structure to store an emuwindow configuration
    struct Config{
        bool    fullscreen;
        int     res_width;
        int     res_height;
    };

    /// Swap buffers to display the next frame
    virtual void SwapBuffers() = 0;

    /**
     * @brief Sets the window title
     * @param title Title to set the window to
     */
    virtual void SetTitle(const char* title) = 0;

    /// Makes the graphics context current for the caller thread
    virtual void MakeCurrent() = 0;

    /// Releases (dunno if this is the "right" word) the GLFW context from the caller thread
    virtual void DoneCurrent() = 0;

    /**
     * @brief gets the window size, used by the renderer to properly scale video output
     * @param width Window width in pixels
     * @param height Window height in pixels
     */
    virtual void GetWindowSize(int &width, int &height) = 0;

    /**
     * @brief Sets the window configuration
     * @param config Configuration to set the window to, includes fullscreen, size, etc
     */
    virtual void SetConfig(Config config) = 0;

protected:
    EmuWindow() {}
    virtual ~EmuWindow() {}

private:
    Config config_;         ///< Internal configuration

    DISALLOW_COPY_AND_ASSIGN(EmuWindow);
};

#endif // __EMUWINDOW_H__
