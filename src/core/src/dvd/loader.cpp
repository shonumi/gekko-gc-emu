/*!
 * Copyright (C) 2005-2012 Gekko / ProjectCafe Emulator
 *
 * \file    loader.cpp
 * \author  Lightning, ShizZy <shizzy247@gmail.com>
 * \date    2012-02-12
 * \brief   Frontend interface for dvd/rom loading
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

#include "common.h"
#include "loader.h"
#include "powerpc/cpu_core.h"

namespace dvd {

/// Loads a ROM to be ran by the emulator
int LoadBootableFile(char* filename) {
    char *ext;
    char ext2[64];
    char str[256], *c;
    char *PipeData;
    u32 FileLoaded = 1;

    std::ifstream ifile(filename);
    if (!ifile) {
        return E_ERR;
    }
    ifile.close();

    c = strrchr(filename, '/');
    if (c) {
        strcpy_s(str, 256, c + 1);
    } else {
        c = strrchr(filename, '\\');
        if (c) {
            strcpy_s(str, 256, c + 1);
        } else {
            strcpy_s(str, 256, filename);
        }
    }
    c = strrchr(str, '.');
    if (c) {
        strcpy_s(ext2, 64, c + 1);
        _strlwr_s(ext2, 64);
        ext = ext2;
    }
    if(cpu->IsCPUCompareActive() == 2)
    {
        PipeData = (char *)malloc(strlen(filename) + 2);
        PipeData[0] = 'F';
        memcpy(&PipeData[1], filename, strlen(filename) + 1);
        cpu->SendPipeData(PipeData, strlen(filename) + 2);
        Sleep(1000);
        free(PipeData);
    }
    if (E_OK == _stricmp(ext, "dol")) {
        LoadDOL(filename);
    } else if (E_OK == _stricmp(ext, "elf")) {
        LoadELF(filename);
    } else if (E_OK == _stricmp(ext, "gcm") || E_OK == _stricmp(ext, "iso")) {
        LoadGCM(filename);
    } else if (E_OK == _stricmp(ext, "dmp")) {
        strcpy_s(ext, 256, c - 3);
        if(E_OK == _stricmp(ext, "gcm.dmp")) {
            dvd::LoadDMP(filename);
        }
    } else {
        LOG_ERROR(TDVD, "Unsupported DVD/ROM type!\n");
        FileLoaded = 0;
        return E_ERR;
    }
    return E_OK;
}

} // namespace