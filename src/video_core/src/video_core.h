/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    video_core.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-08
 * \brief   Main module for new video core
 *
 * \section LICENSE
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

#ifndef VIDEO_COMMON_VIDEO_CORE_H_
#define VIDEO_COMMON_VIDEO_CORE_H_

#include "common.h"
#include "renderer_base.h"

//#define USE_NEW_VIDEO_CORE

#undef LOG_DEBUG
#define LOG_DEBUG(x,y, ...)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Video Core namespace

namespace video_core {

extern RendererBase*   g_renderer;  ///< Renderer plugin 

/// Start the video core
void Start();

/// Initialize the video core
void Init();

/// Shutdown the video core
void ShutDown();

} // namespace

#endif // VIDEO_COMMON_VIDEO_CORE_H_