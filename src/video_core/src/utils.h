/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    utils.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-12-28
 * @brief   Utility functions (in general, not related to emulation) useful for video core
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

#ifndef VIDEO_CORE_UTILS_H_
#define VIDEO_CORE_UTILS_H_

#include "types.h"
#include <string>

namespace video_core {

/// Structure for the TGA texture format (for dumping)
struct TGAHeader {
    char  idlength;
    char  colourmaptype;
    char  datatypecode;
    short int colourmaporigin;
    short int colourmaplength;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};

/**
 * Dumps a texture to TGA
 * @param filename String filename to dump texture to
 * @param width Width of texture in pixels
 * @param height Height of texture in pixels
 * @param raw_data Raw RGBA8 texture data to dump
 * @todo This should be moved to some general purpose/common code
 */
void DumpTGA(std::string filename, int width, int height, u8* raw_data);

} // namespace

#endif // VIDEO_CORE_UTILS_H_
