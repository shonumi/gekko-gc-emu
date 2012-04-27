/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    fifo.h
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

EmuWindow_GLFW::EmuWindow_GLFW()
{
    if(glfwInit() != GL_TRUE) {
        LOG_ERROR(TVIDEO, "Failed to initialize GLFW! Exiting...");
        exit(E_ERR);
    }
    //glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16); // 2X AA
    //glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    //glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
    //glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if(glfwOpenWindow(0, 0, 0, 0, 0, 0, 0, 0, GLFW_WINDOW) != GL_TRUE) {
        LOG_ERROR(TVIDEO, "Failed to open GLFW window! Exiting...");
        glfwTerminate();
        exit(E_ERR);
    }
    LOG_NOTICE(TVIDEO, "OpenGL Context: %d.%d initialzed ok\n", 
        glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR), 
        glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR));

    glfwSetWindowTitle("gekko-glfw");
}
EmuWindow_GLFW::~EmuWindow_GLFW()
{
    // TODO..
}

void EmuWindow_GLFW::SwapBuffers()
{
    glfwSwapBuffers();
}
void EmuWindow_GLFW::SetTitle(const char* title)
{
    glfwSetWindowTitle(title);
}