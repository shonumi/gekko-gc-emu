/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    emuwindow_glfw.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-04-20
 * @brief   Implementation implementation of EmuWindow class for GLFW
 *
 * @section LICENSE
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */

#ifndef VIDEO_CORE_EMUWINDOW_GLFW_
#define VIDEO_CORE_EMUWINDOW_GLFW_

#include <GL/glew.h>
#include <GL/glfw3.h>

#include "video/emuwindow.h"

class EmuWindow_GLFW : public EmuWindow {
public:
    EmuWindow_GLFW();
    ~EmuWindow_GLFW();

    /// Swap buffers to display the next frame
    void SwapBuffers();

	/// Polls window events
	void PollEvents();

    /**
     * @brief Sets the window title
     * @param title Title to set the window to
     */
    void SetTitle(const char* title);

    /// Makes the graphics context current for the caller thread
    void MakeCurrent();
    
    /// Releases (dunno if this is the "right" word) the GLFW context from the caller thread
    void DoneCurrent();

    /**
     * @brief gets the window size, used by the renderer to properly scale video output
     * @param width Window width in pixels
     * @param height Window height in pixels
     */
    void GetWindowSize(int& width, int& height);

    /**
     * @brief Sets the window configuration
     * @param config Configuration to set the window to, includes fullscreen, size, etc
     */
    void SetConfig(Config config);

	GLFWwindow render_window_;      ///< Internal GLFW render window

private:
    
};

#endif // VIDEO_CORE_EMUWINDOW_GLFW_
