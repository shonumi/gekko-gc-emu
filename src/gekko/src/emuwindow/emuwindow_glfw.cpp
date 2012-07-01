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

#include "common.h"
#include "emuwindow_glfw.h"

/// EmuWindow_GLFW constructor
EmuWindow_GLFW::EmuWindow_GLFW()
{
    if(glfwInit() != GL_TRUE) {
        LOG_ERROR(TVIDEO, "Failed to initialize GLFW! Exiting...");
        exit(E_ERR);
    }
    
    glfwOpenWindowHint(GLFW_WINDOW_RESIZABLE, GL_FALSE);
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

    render_window_ = glfwOpenWindow(640, 480, GLFW_WINDOWED, "gekko-glfw3", 0);
    DoneCurrent();
}

/// EmuWindow_GLFW destructor
EmuWindow_GLFW::~EmuWindow_GLFW() {
    glfwTerminate();
}

/// Swap buffers to display the next frame
void EmuWindow_GLFW::SwapBuffers() {
    glfwPollEvents();
    glfwSwapBuffers();
}

/**
 * @brief Sets the window title
 * @param title Title to set the window to
 */
void EmuWindow_GLFW::SetTitle(const char* title) {
    glfwSetWindowTitle(render_window_, title);
}

/// Makes the GLFW OpenGL context current for the caller thread
void EmuWindow_GLFW::MakeCurrent() {
    glfwMakeContextCurrent(render_window_);
}

/// Releases (dunno if this is the "right" word) the GLFW context from the caller thread
void EmuWindow_GLFW::DoneCurrent() {
    glfwMakeContextCurrent(NULL);
}

/**
 * @brief gets the window size, used by the renderer to properly scale video output
 * @param width Window width in pixels
 * @param height Window height in pixels
 */
void EmuWindow_GLFW::GetWindowSize(int &width, int &height) {
}

/**
 * @brief Sets the window configuration
 * @param config Configuration to set the window to, includes fullscreen, size, etc
 */
void EmuWindow_GLFW::SetConfig(EmuWindow::Config config) {
}
