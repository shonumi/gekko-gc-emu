/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    elf.h
 * \author  Lightning
 * \date    2006-01-01
 * \brief   Interface for loading an ELF
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

#ifndef CORE_DVD_ELF_H_
#define CORE_DVD_ELF_H_

#include "realdvd.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

DEFRealDVDRead(ELFDVDRead);
DEFRealDVDSeek(ELFDVDSeek);
DEFRealDVDOpen(ELFDVDOpen);
DEFRealDVDGetFileSize(ELFDVDGetFileSize);
DEFRealDVDClose(ELFDVDClose);
DEFRealDVDGetPos(ELFDVDGetPos);
DEFRealDVDChangeDir(ELFDVDChangeDir);

u32 ELF_CreateFileStructure(char *ELFFileName);

} // namespace

#endif // CORE_DVD_ELF_H_