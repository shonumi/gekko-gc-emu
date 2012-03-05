 /*!
 * Copyright (C) 2005-2012 Gekko / ProjectCafe Emulator
 *
 * \file    dol.cpp
 * \author  Lightning, ShizZy <shizzy247@gmail.com>
 * \date    2006-01-01
 * \brief   Interface for loading a DOL
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
#include "memory.h"
#include "hw/hw.h"
#include "powerpc/cpu_core.h"
#include "dvd/realdvd.h"
#include "hle/hle.h"
#include "boot/bootrom.h"
#include "elf.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

char    DOLCurrentDir[256];

DEFRealDVDRead(DOLDVDRead)
{
    if(FilePtr == REALDVD_LOWLEVEL) {
        return 0;
    }

    return fread(MemPtr, Len, 1, (FILE *)FilePtr);
}

DEFRealDVDSeek(DOLDVDSeek)
{
    int fSeekType;

    if(FilePtr == REALDVD_LOWLEVEL)
        return 0;

    switch(SeekType)
    {
    case REALDVDSEEK_START:
        fSeekType = SEEK_SET;
        break;
    case REALDVDSEEK_CUR:
        fSeekType = SEEK_CUR;

        break;
    case REALDVDSEEK_END:
        fSeekType = SEEK_END;
        break;
    };

    return fseek((FILE *)FilePtr, SeekPos, fSeekType);
}

DEFRealDVDOpen(DOLDVDOpen)
{
    char FileNameBuff[256];
    char TempDir[256];
    int i = 0;
    u32 NewHandle;

    //adjust the filename if a / is at the beginning
    if(filename[0] == '/') i=1;
    sprintf(FileNameBuff, "dvdroot\\%s", &filename[i]);

    //convert the / to \'s
    for(i=0;i<strlen(FileNameBuff);i++)
    {
        if(FileNameBuff[i]=='/') FileNameBuff[i]='\\';
    }

    //change the directory
    getcwd(TempDir, 256);
    chdir(DOLCurrentDir);

    //open up the file
    NewHandle = ((u8*)fopen(FileNameBuff, "rb") - (u8*)NULL);

    //put the directory back
    chdir(TempDir);
    return NewHandle;
}

DEFRealDVDGetFileSize(DOLDVDGetFileSize)
{
    //return GetFileSize((HANDLE)FilePtr, NULL);
    u32		LastPos;
    u32		FileSize;
    LastPos = ftell((FILE *)FilePtr);
    fseek((FILE *)FilePtr, 0, SEEK_END);
    FileSize = ftell((FILE *)FilePtr);
    fseek((FILE *)FilePtr, LastPos, SEEK_SET);
    return FileSize;
}

DEFRealDVDClose(DOLDVDClose)
{
    if(FilePtr != REALDVD_LOWLEVEL)
        fclose((FILE *)FilePtr);
    else
    {
        //cleanup
        ResetRealDVD();
    }

    return 1;
}

DEFRealDVDGetPos(DOLDVDGetPos)
{
    return ftell((FILE *)FilePtr);
}

DEFRealDVDChangeDir(DOLDVDChangeDir)
{
    char	TempCurDir[256];
    char	NewDir[256];
    int		i;

    //store the current path
    getcwd(TempCurDir, 256);

    //get the new directory
    strcpy(NewDir, ChangeDirPath);

    //swap / to \ in the new path
    for(i=0; i < strlen(NewDir); i++)
    {
        if(NewDir[i] == '/')
            NewDir[i] = '\\';
    }

    //now change to it
    chdir(DOLCurrentDir);
    i = chdir(NewDir);

    //if no error, then save it
    if(!i) getcwd(DOLCurrentDir, 256);

    //swap the directory back for the app
    chdir(TempCurDir);
    return i;
}

/*!
 * \brief Loads a section of a DOL binary
 * \param f File pointer to read from
 * \param srcaddr Source address
 * \param dstaddr Destination address
 * \param len Length
 */
bool LoadDOLSection(FILE* f, u32 srcaddr, u32 dstaddr, u32 len)
{
    u8 *FileData;
    u32	x;

    if(!f)
    {
        LOG_ERROR(TDVD, "Invalid Loaded DOL Has No Size!");
        return false;
    }

    if(len == 0)
        return true;

    fseek(f,BSWAP32(srcaddr),SEEK_SET);

    len = BSWAP32(len);
    FileData = (u8*)malloc(len);
    if(FileData)
    {
        fread(FileData, len, 1, f);

        dstaddr = BSWAP32(dstaddr);
        for(x = 0; x < len; x++)
        {
            Memory_Write8(dstaddr, (u32)FileData[x]);
            dstaddr++;
        }

        LOG_ERROR(TDVD, "DOL %08X bytes copied to address %08X",len,dstaddr);

        free(FileData);
    }

    return true;
}

/// Load a DOL (GameCube binary file)
int LoadDOL(char *filename) {
    u32 FSTStart;
    DOLHeader dol;
    Memory_Open();

    FILE * f = fopen(filename,"rb");

    fread(&dol, 1, sizeof(DOLHeader), f);

    for(int i = 0; i < DOL_NUMTEXT; i++)
    {
        if(!LoadDOLSection(f, dol.text_offset[i], dol.text_address[i], dol.text_size[i])) {
            LOG_ERROR(TDVD, "Unable to load DOL text section!");
        }
    }

    for(int i = 0; i < DOL_NUMDATA; i++)
    {
        if(!LoadDOLSection(f, dol.data_offset[i], dol.data_address[i], dol.data_size[i])) {
            LOG_ERROR(TDVD, "Unable to load DOL data section!");
        }
    }
    fclose(f);

    // Setup the files
    FSTStart = ELF_CreateFileStructure(filename);

    // Setup the RealDVD entries
    RealDVDOpen = &ELFDVDOpen;
    RealDVDSeek = &ELFDVDSeek;
    RealDVDRead = &ELFDVDRead;
    RealDVDGetFileSize = &ELFDVDGetFileSize;
    RealDVDClose = &ELFDVDClose;
    RealDVDGetPos = &ELFDVDGetPos;
    RealDVDChangeDir = &ELFDVDChangeDir;

    Bootrom(FSTStart);

    Flipper_Open();
    cpu->Open(BSWAP32(dol.entry_point));

    HLE_ScanForPatches();
    return E_OK;
}

} // namespace

