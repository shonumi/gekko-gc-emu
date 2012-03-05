/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    gcm.cpp
 * \author  Lightning
 * \date    2006-01-01
 * \brief   Interface for loading a GCM GameCube DVD image
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
#include "realdvd.h"
#include "powerpc/cpu_core.h"
#include "boot/bootrom.h"
#include "boot/apploader.h"
#include "hw/hw.h"
#include "hle/hle.h"
#include "gcm.h"
#include "memory.h"
#include "core.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

HANDLE  FileHandle = INVALID_HANDLE_VALUE;
HANDLE  DumpFileHandle = INVALID_HANDLE_VALUE;

char	g_current_game_name[992];
char	g_current_game_crc[7];

//all filenames in the FST
char *FileNames;

//pointer to the FST
GCMFileData *	FST;

//pointers to open files
GCMFileInfo *	FilePtrs;
GCMFileInfo *	LowLevelPtr;
GCMFileData *	GCMCurDir;

u32		DumpGCMBlockReads = 0;
char	DumpDirectory[1024] = ".";

/// Gets the banner checksum
u8 GetBnrChecksum(void *banner)
{
    DVDBanner*  bnr  = (DVDBanner *)banner;
    DVDBanner2* bnr2 = (DVDBanner2*)banner;
    u8*         buf;
    u32         sum  = 0;

    if(BSWAP32(bnr->id) == DVD_BANNER_ID) // USA/JAP
    {
        buf = (u8 *)bnr;
        for(int i=0; i<sizeof(DVDBanner); i++)
        {
            sum += buf[i];
        }
    }
    else if(BSWAP32(bnr->id) == DVD_BANNER_ID2) // USA/JAP// EUR
    {
        buf = (u8 *)bnr2;
        for(int i=0; i<sizeof(DVDBanner2); i++)
        {
            sum += buf[i];
        }
    }

    return (u8)sum;
}

DEFRealDVDRead(GCMDVDRead)
{
    DWORD			ReadLen;
    DWORD			WriteLen;
    GCMFileInfo *	GCMFilePtr;

    //if the file handle is invalid then exit
    if(FileHandle == INVALID_HANDLE_VALUE)
        return 0;

    if(FilePtr == 0)
        return 0;

    //if the special id, update the file handle to the first entry
    if(FilePtr == REALDVD_LOWLEVEL)
        GCMFilePtr = LowLevelPtr;
    else
        GCMFilePtr = (GCMFileInfo *)FilePtr;

    if(GCMFilePtr->ID != GCMFILEID)
        return 0;

    //read from the file
    SetFilePointer(FileHandle, GCMFilePtr->FileData->DiskAddr + GCMFilePtr->CurPos, NULL, FILE_BEGIN);

    //if the length puts the cursor past the end of the file, then adjust the length
    if((GCMFilePtr->CurPos + Len) > GCMFilePtr->FileData->FileSize)
        Len = GCMFilePtr->FileData->FileSize - GCMFilePtr->CurPos;

    //read from the file
    if(!ReadFile(FileHandle, MemPtr, Len, &ReadLen, 0))
    {
        LOG_NOTICE(TDVD, "Reading invalid area of file!\n");
        return 0;
    }

    if(DumpGCMBlockReads)
        WriteFile(DumpFileHandle, MemPtr, Len, &WriteLen, 0);

    //adjust the current pointer
    GCMFilePtr->CurPos += ReadLen;
    return ReadLen;
}

DEFRealDVDSeek(GCMDVDSeek)
{
    GCMFileInfo *	GCMFilePtr;
    long			NewPos;

    if(FileHandle == INVALID_HANDLE_VALUE)
        return 0;

    if(FilePtr == 0)
        return 0;

    //if the special id, update the file handle to the first entry
    if(FilePtr == REALDVD_LOWLEVEL)
        GCMFilePtr = LowLevelPtr;
    else
        GCMFilePtr = (GCMFileInfo *)FilePtr;

    if(GCMFilePtr->ID != GCMFILEID)
        return 0;

    //if the length puts the cursor past the end of the file, then adjust the length
    switch(SeekType)
    {
    case REALDVDSEEK_START:
        NewPos = (long)SeekPos;
        break;
    case REALDVDSEEK_CUR:
        NewPos = GCMFilePtr->CurPos + (long)SeekPos;
        break;
    case REALDVDSEEK_END:
        NewPos = GCMFilePtr->FileData->FileSize - (long)SeekPos;
        break;
    };

    //verify the new pointer is valid
    if(NewPos < 0)
        return -1;
    else if((u32)NewPos > GCMFilePtr->FileData->FileSize)
        NewPos = GCMFilePtr->FileData->FileSize;

    //set the file pointer
    NewPos = SetFilePointer(FileHandle, GCMFilePtr->FileData->DiskAddr + NewPos, NULL, FILE_BEGIN);

    //adjust the struct
    NewPos -= GCMFilePtr->FileData->DiskAddr;
    GCMFilePtr->CurPos = NewPos;
    return NewPos;
}

DEFRealDVDClose(GCMDVDClose)
{
    GCMFileInfo *		GCMFilePtr;
    GCMFileInfo *		OldGCMFilePtr;

    if(FileHandle == INVALID_HANDLE_VALUE)
        return 0;

    if(FilePtr == 0)
        return 0;

    //if the special id, update the file handle to the first entry
    if(FilePtr == REALDVD_LOWLEVEL)
    {
        //cleanup
        CloseHandle(FileHandle);

        if(DumpGCMBlockReads)
            CloseHandle(DumpFileHandle);

        ResetRealDVD();

        //remove all file pointers
        while(FilePtrs)
        {
            OldGCMFilePtr = FilePtrs;
            FilePtrs = FilePtrs->NextPtr;
            free(OldGCMFilePtr);
        }
        free(LowLevelPtr);
        LowLevelPtr = NULL;

        //wipe out the FST data
        free(FileNames);
        free(FST->FileList);
        free(FST);
        free(LowLevelPtr);
        FST = NULL;

        FileHandle = INVALID_HANDLE_VALUE;
        return 0;
    }
    else
        GCMFilePtr = (GCMFileInfo *)FilePtr;

    if(GCMFilePtr->ID != GCMFILEID)
        return 0;

    //remove the entry from the tree
    if(GCMFilePtr->NextPtr)
        GCMFilePtr->NextPtr->PrevPtr = GCMFilePtr->PrevPtr;

    if(GCMFilePtr->PrevPtr)
        GCMFilePtr->PrevPtr->NextPtr = GCMFilePtr->NextPtr;
    else
        FilePtrs = GCMFilePtr->NextPtr;

    //unload the memory
    free(GCMFilePtr);
    return 1;
}

DEFRealDVDGetFileSize(GCMDVDGetFileSize)
{
    GCMFileInfo *		GCMFilePtr;

    if(FileHandle == INVALID_HANDLE_VALUE)
        return 0;

    if(FilePtr == 0)
        return 0;

    //if the special id, update the file handle to the first entry
    if(FilePtr == REALDVD_LOWLEVEL)
        GCMFilePtr = LowLevelPtr;
    else
        GCMFilePtr = (GCMFileInfo *)FilePtr;

    if(GCMFilePtr->ID != GCMFILEID)
        return 0;

    return GCMFilePtr->FileData->FileSize;
}

DEFRealDVDGetPos(GCMDVDGetPos)
{
    GCMFileInfo *		GCMFilePtr;

    if(FileHandle == INVALID_HANDLE_VALUE)
        return 0;

    if(FilePtr == 0)
        return 0;

    //if the special id, update the file handle to the first entry
    if(FilePtr == REALDVD_LOWLEVEL)
        GCMFilePtr = LowLevelPtr;
    else
        GCMFilePtr = (GCMFileInfo *)FilePtr;

    if(GCMFilePtr->ID != GCMFILEID)
        return 0;

    return GCMFilePtr->CurPos;
}

GCMFileData *FindFSTEntry(GCMFileData *CurEntry, char *Filename)
{
    u32				i;
    GCMFileData *	NewEntry;

    //if we hit the end of the filename then return the current entry
    if(Filename[0] == 0x00)
        return CurEntry;

    //if the current entry is not a directory then return null
    if(CurEntry->IsDirectory != 1)
        return NULL;

    //go thru the list to find a match
    for(i=0; i < CurEntry->FileCount; i++)
    {
        NewEntry = &CurEntry->FileList[i];
        if(strcmp(NewEntry->Filename, Filename) == 0)
            return FindFSTEntry(NewEntry, &Filename[strlen(Filename) + 1]);
    }

    //did not find an entry, return null
    return NULL;
}

GCMFileData *ChangeDirEntry(GCMFileData *CurEntry, char *Filename)
{
    u32				i;
    GCMFileData *	NewEntry;

    //if the entry is null, exit
    if(!CurEntry)
        return NULL;

    //if we hit the end of the filename then return the current entry
    if(Filename[0] == 0x00)
    {
        //verify a / wasnt specified for root
        if(Filename[1] == 0x00)
            return CurEntry;
        else
            return FindFSTEntry(FST, &Filename[1]);
    }

    //if the current entry is not a directory then return null
    if(CurEntry->IsDirectory != 1)
        return NULL;

    //if . then goto current
    if(strcmp(Filename,".") == 0)
        return FindFSTEntry(CurEntry, &Filename[strlen(Filename) + 1]);
    else if(strcmp(Filename, "..") == 0)
        return FindFSTEntry(CurEntry->Parent, &Filename[strlen(Filename) + 1]);
    else
    {
        //go thru the list to find a match
        for(i=0; i < CurEntry->FileCount; i++)
        {
            NewEntry = &CurEntry->FileList[i];
            if(strcmp(NewEntry->Filename, Filename) == 0)
                return FindFSTEntry(NewEntry, &Filename[strlen(Filename) + 1]);
        }
    }

    //did not find an entry, return null
    return NULL;
}

DEFRealDVDChangeDir(GCMDVDChangeDir)
{
    char	NewDir[256];
    size_t	i;
    GCMFileData *	NewPath;

    //copy the directory passed in
    memset(NewDir, 0, sizeof(NewDir));
    strcpy(NewDir, ChangeDirPath);

    //go thru the directory and change / to .
    for(i=0; i < strlen(ChangeDirPath); i++)
        if(NewDir[i] == '/') NewDir[i] = 0x00;

    NewPath = ChangeDirEntry(GCMCurDir, NewDir);
    if(NewPath)
        GCMCurDir = NewPath;

    return (NewPath != NULL);
}

DEFRealDVDOpen(GCMDVDOpen)
{
    GCMFileInfo *	GCMFilePtr;
    GCMFileData *	GCMFileList;
    u32				i;
    char *			FindFilename;
    GCMFileData *	GCMOpenDir;

    //copy the filename then split it up on the nulls
    FindFilename = (char *)malloc(strlen(filename) + 2);
    memcpy(FindFilename, filename, strlen(filename) + 1);
    FindFilename[strlen(filename) + 1] = 0;

    //split the filename up
    for(i=0; i < strlen(filename); i++)
    {
        //set the / or \ to a null char
        if((FindFilename[i] == '/') || (FindFilename[i] == '\\'))
            FindFilename[i] = 0x00;

        //if a letter, make uppercase
        if(FindFilename[i] >= 'a' && FindFilename[i] <= 'z')
            FindFilename[i] &= 0xDF;
    }

    //if the first char is a slash then reset back to the root dir for opening
    if(FindFilename[0] == 0x00)
    {
        GCMOpenDir = FST;
        i = 1;		//skip the first character in the filename
    }
    else
    {
        GCMOpenDir = GCMCurDir;
        i = 0;		//do not skip the first character in the filename
    }

    //find the file requested, the path starts with /
    GCMFileList = FindFSTEntry(GCMOpenDir, &FindFilename[i]);
    if(!GCMFileList)
        return 0;

    //allocate memory for the structure
    GCMFilePtr = (GCMFileInfo *)malloc(sizeof(GCMFileInfo));
    memset((void *)GCMFilePtr, 0, sizeof(GCMFileInfo));

    //setup the structure
    GCMFilePtr->ID = GCMFILEID;
    GCMFilePtr->FileData = GCMFileList;

    //add it to the beginning of the tree
    GCMFilePtr->NextPtr = FilePtrs;
    if(FilePtrs)
        FilePtrs->PrevPtr = GCMFilePtr;
    FilePtrs = GCMFilePtr;

    //return the pointer as the file ID
    return (u32)GCMFilePtr;
}

void ParseFSTTree(GCMFileData *FSTEntry, GCMFileData *RootFSTEntry, GCMFST **CurEntry, char *Filenames, u32 *Count, GCMFileData *ParentFST)
{
    u32				i;
    u32				OldCount;

    FSTEntry->DiskAddr = (*CurEntry)->DiskAddr;
    FSTEntry->Filename = &Filenames[(*CurEntry)->NameOffset & 0x00FFFFFF];
    FSTEntry->Parent = ParentFST;

    if((*CurEntry)->NameOffset & 0xFF000000)
    {
        FSTEntry->IsDirectory = 1;
        FSTEntry->FileList = &RootFSTEntry[(*Count)];
        FSTEntry->FileCount = (*CurEntry)->FileSize;

        (*Count) += (*CurEntry)->FileSize;
        OldCount = (*CurEntry)->FileSize;

        for(i=0; i < OldCount; i++)
        {
            (*CurEntry)++;
            ParseFSTTree(&FSTEntry->FileList[i], RootFSTEntry, CurEntry, Filenames, Count, FSTEntry);
        }
    }
    else
        FSTEntry->FileSize = (*CurEntry)->FileSize;
}

void DumpFSTEntry(HANDLE DumpHand, GCMFileData *Entry, u32 FileCount, u32 Level)
{
    u32		i;
    u32		x;
    char	FileBuff[512];

    if(Entry == NULL)
        return;

    for(i=0; i < FileCount; i++)
    {
        memset(FileBuff, 0, sizeof(FileBuff));

        for(x=0;x < Level; x++)
            sprintf(FileBuff, "   %s", FileBuff);
        sprintf(FileBuff, "%s %s %d [0x%08X] [0x%08X]\r\n", FileBuff, Entry[i].Filename, Entry[i].FileSize, &Entry[i], (u32)(&Entry[i]) + sizeof(GCMFileData));

        WriteFile(DumpHand, FileBuff, strlen(FileBuff), (LPDWORD)&x, 0);
        if(Entry[i].IsDirectory)
            DumpFSTEntry(DumpHand, Entry[i].FileList, Entry[i].FileCount, Level+1);
    }
}

void DumpFST(char *filename, GCMFileData *Entry)
{
    HANDLE	DumpHand;

    DumpHand = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    DumpFSTEntry(DumpHand, Entry->FileList, Entry->FileCount, 0);
    CloseHandle(DumpHand);
}

u32 AdjustFSTCounts(GCMFST *CurGCMFSTData, u32 *CurIndex, u32 LastIndex)
{
    u32	TotalCount = 0;
    u32	OldIndex;

    //adjust all the counts of the directories to allow easier processing
    for(; *CurIndex < LastIndex;)
    {
        TotalCount++;

        //if a directory then adjust it's counts
        if(CurGCMFSTData[*CurIndex].NameOffset & 0xFF000000)
        {
            OldIndex = *CurIndex;
            (*CurIndex)++;
            CurGCMFSTData[OldIndex].FileSize = AdjustFSTCounts(CurGCMFSTData, CurIndex, CurGCMFSTData[OldIndex].FileSize - 1);
        }
        else
            (*CurIndex)++;
    }

    return TotalCount;
}

int LoadGCM(char *filename)
{
    GCMFSTHeader	FSTInfo;
    DWORD			BytesRead;
    u32				TempData;
    u32				FileCount;
    u32				i;
    u32				x;
    GCMFST *		GCMFSTData;
    GCMFST *		CurGCMFSTData;
    u32				AppLoaderHeader[8];
    u8 				Banner[0x1960];
    u8 				Banner2[0x1820 + (0x140 * 6)];
    char			BannerFilename[] = "OPENING.BNR\0\0";
    u8				BannerCRC = 0x00;
    GCMFileData *	BannerData;
    char *			dumpfilename;
    char *			FileSlash;
    char			Header[SIZE_OF_GCM_HEADER];

    //if a file is already open, fail
    if(FileHandle != INVALID_HANDLE_VALUE) {
        return E_ERR;
    }

    //open it up
    FileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
    if(FileHandle == INVALID_HANDLE_VALUE) {
        return E_ERR;
    }
    if(DumpGCMBlockReads) {
        dumpfilename = (char *)malloc(1024);

        if(strlen(DumpDirectory)) {
            strcpy(dumpfilename, DumpDirectory);
            if(DumpDirectory[strlen(DumpDirectory) - 1] != '\\')
                strcat(dumpfilename, "\\");

            FileSlash = strrchr(filename, '\\');
            strcat(dumpfilename, &FileSlash[1]);
        } else {
            strcpy(dumpfilename, filename);
        }

        strcat(dumpfilename, ".dmp");

        DumpFileHandle = CreateFile(dumpfilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
        free(dumpfilename);
    }

    //setup the MSR
    set_ireg_MSR(MSR_BIT_DR | MSR_BIT_IR | MSR_BIT_FP);

    Memory_Open();

    //read the first 32 bytes into the root memory area
    ReadFile(FileHandle, &Mem_RAM[0], 32, &BytesRead, 0);

    //get a copy of the CRC into the header
    memcpy(Header, &Mem_RAM[0], 32);

    if(DumpGCMBlockReads) {
        WriteFile(DumpFileHandle, &Mem_RAM[0], 32, &BytesRead, 0);
    }

    //swap the memory around
    for(i = 0; i < (32 >> 2); i++) {
        *(u32 *)(&Mem_RAM[i * 4]) = BSWAP32(*(u32 *)(&Mem_RAM[i * 4]));
    }

    //read the game name, make sure the last byte is null terminated
    //0x400 - 0x20 = 3E0
    ReadFile(FileHandle, g_current_game_name, 0x3E0, &BytesRead, 0);

    if(DumpGCMBlockReads) {
        WriteFile(DumpFileHandle, g_current_game_name, 0x3E0, &BytesRead, 0);
    }
    core::g_config_manager->ReloadGameConfig(Header);

    // Print loading message
    int npatches = 0;
    for (npatches = 0; npatches < MAX_PATCHES_PER_GAME; npatches++) {
        if (!common::g_config->patches(npatches).address) {
            break;
        }
    }
    char linestr[81] = "**********************************************************************";
    char hle_enable_str[10] = "disabled";
    if (common::g_config->enable_hle()) {
        sprintf_s(hle_enable_str, 10, "enabled");
    }
    LOG_NOTICE(TDVD, linestr);
    LOG_NOTICE(TDVD, "Loading \"%s\"", g_current_game_name);
    LOG_NOTICE(TDVD, linestr); 
    LOG_NOTICE(TDVD, "GameID:\t%s", Header);
    LOG_NOTICE(TDVD, "Patches:\t%d (%s)", npatches, hle_enable_str);
    LOG_NOTICE(TDVD, linestr);

    //see if we are in pal mode
    if(Memory_Read8(0x80000003) == (u8)'P') Memory_Write32(0x800000CC, 1);

    //read the FST
    if(SetFilePointer(FileHandle, 0x424, NULL, SEEK_SET) != 0x424)
    {
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    //get the FST info header
    ReadFile(FileHandle, &FSTInfo, sizeof(FSTInfo), &BytesRead, 0);
    if(BytesRead != sizeof(FSTInfo))
    {
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    if(DumpGCMBlockReads)
        WriteFile(DumpFileHandle, &FSTInfo, sizeof(FSTInfo), &BytesRead, 0);

    //swap the FST info
    FSTInfo.Offset = BSWAP32(FSTInfo.Offset);
    FSTInfo.Size = BSWAP32(FSTInfo.Size);
    FSTInfo.MaxSize = BSWAP32(FSTInfo.MaxSize);
    FSTInfo.MemLocation = BSWAP32(FSTInfo.MemLocation) + RAM_24MB - (4*1024*1024);	//last 4 megs of mem

    Memory_Write32(0x80000038, FSTInfo.MemLocation);
    Memory_Write32(0x8000003C, FSTInfo.MaxSize);

    //seek to the position of the FST + 8 bytes to get the number of files
    if(SetFilePointer(FileHandle, FSTInfo.Offset + 8, NULL, SEEK_SET) != (FSTInfo.Offset + 8))
    {
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    //read 4 bytes for the number of files
    ReadFile(FileHandle, &FileCount, 4, &BytesRead, 0);
    if(BytesRead != 4)
    {
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    if(DumpGCMBlockReads)
        WriteFile(DumpFileHandle, &FileCount, 4, &BytesRead, 0);

    //go back. the first entry is empty but tells the number of files
    if(SetFilePointer(FileHandle, FSTInfo.Offset, NULL, SEEK_SET) != FSTInfo.Offset)
    {
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    //allocate memory for the FST info and filenames
    FileCount = BSWAP32(FileCount);
    GCMFSTData = (GCMFST *)malloc((FileCount+1) * sizeof(GCMFST));
    FileNames = (char *)malloc(FSTInfo.Size - (FileCount * sizeof(GCMFST)));
    memset(&GCMFSTData[FileCount], 0, sizeof(GCMFST));

    //read the data
    ReadFile(FileHandle, GCMFSTData, FileCount * sizeof(GCMFST), &BytesRead, 0);
    if(BytesRead != (FileCount * sizeof(GCMFST)))
    {
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    if(DumpGCMBlockReads)
        WriteFile(DumpFileHandle, GCMFSTData, FileCount * sizeof(GCMFST), &BytesRead, 0);

    //make a copy of it to memory
    for(i = 0; i < (BytesRead >> 2); i++)
        Memory_Write32(FSTInfo.MemLocation + (i * 4), *(u32 *)&(((u8 *)GCMFSTData)[i * 4]));

    for(i = i * 4; i < BytesRead; i++)
        Memory_Write8(FSTInfo.MemLocation + i, *(u32 *)&((u8 *)GCMFSTData)[i]);

    //swap all of the numerical FST data in the GCM data
    for(i=1; i < FileCount; i++)
    {
        GCMFSTData[i].DiskAddr = BSWAP32(GCMFSTData[i].DiskAddr);
        GCMFSTData[i].FileSize = BSWAP32(GCMFSTData[i].FileSize);
        GCMFSTData[i].NameOffset = BSWAP32(GCMFSTData[i].NameOffset);
    }

    TempData = FSTInfo.Size - (FileCount * sizeof(GCMFST));
    ReadFile(FileHandle, FileNames, TempData, &BytesRead, 0);
    if(BytesRead != TempData)
    {
        CloseHandle(FileHandle);
        free(FileNames);
        free(GCMFSTData);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    if(DumpGCMBlockReads)
        WriteFile(DumpFileHandle, FileNames, TempData, &BytesRead, 0);

    //copy the filenames
    x = FSTInfo.MemLocation + (FileCount * sizeof(GCMFST));
    for(i = 0; i < (TempData >> 2); i++)
        Memory_Write32(x + (i * 4), *(u32 *)&FileNames[i * 4]);

    for(i = (i * 4); i < FileCount; i++)
        Memory_Write8(x + i, *(u32 *)&FileNames[i]);

    LOG_NOTICE(TDVD, "FST Data loaded 0x%08X bytes to 0x%08X. File names @ 0x%08X\n", FSTInfo.Size, FSTInfo.MemLocation, FSTInfo.MemLocation + (FileCount * sizeof(GCMFST)));

    //	memcpy(MEMPTR32(FSTInfo.MemLocation + (FileCount * sizeof(GCMFST))), FileNames, TempData);

    //make all files uppercase
    for(i=0; i < TempData; i++)
    {
        if(FileNames[i] >= 'a' && FileNames[i] <= 'z') {
            FileNames[i] &= 0xDF;
        }
    }

    FST = (GCMFileData *)malloc(sizeof(GCMFileData));
    if (!FST) {
        CloseHandle(FileHandle);
        free(FileNames);
        free(GCMFSTData);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    memset(FST, 0, sizeof(GCMFileData));

    if (FileCount) {
        FileCount--;
    }

    //setup the root node
    FST->FileCount = FileCount;
    FST->FileList = (GCMFileData *)malloc(FileCount * sizeof(GCMFileData));
    memset(FST->FileList, 0, FileCount * sizeof(GCMFileData));
    FST->FileSize = GetFileSize(FileHandle, NULL);
    FST->DiskAddr = 0;
    FST->Filename = NULL;
    FST->IsDirectory = 1;

    if(DumpGCMBlockReads) {
        WriteFile(DumpFileHandle, &FST->FileSize, sizeof(FST->FileSize), &BytesRead, 0);
    }

    //setup the file count
    i = 0;
    FST->FileCount = AdjustFSTCounts(&GCMFSTData[1], &i, FileCount);

    //now, parse up the FST info into the tree
    TempData = FST->FileCount;
    CurGCMFSTData = &GCMFSTData[1];
    FileCount=0;
    for(i=0; i < FST->FileCount; i++) {
        ParseFSTTree(&FST->FileList[i], FST->FileList, &CurGCMFSTData, FileNames, &TempData, FST);
        CurGCMFSTData++;
    }

    free(GCMFSTData);

    //set the current directory to the root
    GCMCurDir = FST;

    //setup the root file entry
    LowLevelPtr = (GCMFileInfo *)malloc(sizeof(GCMFileInfo));
    memset(LowLevelPtr, 0, sizeof(GCMFileInfo));
    LowLevelPtr->ID = GCMFILEID;
    LowLevelPtr->FileData = FST;
    LowLevelPtr->PrevPtr = NULL;
    LowLevelPtr->NextPtr = NULL;

    //setup the RealDVD entries
    RealDVDOpen = &GCMDVDOpen;
    RealDVDSeek = &GCMDVDSeek;
    RealDVDRead = &GCMDVDRead;
    RealDVDGetFileSize = &GCMDVDGetFileSize;
    RealDVDClose = &GCMDVDClose;
    RealDVDGetPos = &GCMDVDGetPos;
    RealDVDChangeDir = &GCMDVDChangeDir;

    //find the opening.bnr
    BannerData = FindFSTEntry(FST, BannerFilename);

    //Check if the banner is at least 6,496 bytes and do additional check for BNR2 integrity.
    //We only grab the first one....
    if(BannerData) {
        //read the banner
        SetFilePointer(FileHandle, BannerData->DiskAddr, 0, SEEK_SET);
        if (BannerData->FileSize == sizeof(Banner)) {
            ReadFile(FileHandle, Banner, BannerData->FileSize, &BytesRead, 0);

            if(DumpGCMBlockReads)
                WriteFile(DumpFileHandle, Banner, BannerData->FileSize, &BytesRead, 0);

            BannerCRC = GetBnrChecksum(Banner);
        } else if (BannerData->FileSize > sizeof(Banner) && (BannerData->FileSize - 0x1820) % 0x140 == 0x00) {
            ReadFile(FileHandle, Banner2, BannerData->FileSize, &BytesRead, 0);

            if(DumpGCMBlockReads)
                WriteFile(DumpFileHandle, Banner2, BannerData->FileSize, &BytesRead, 0);

            BannerCRC = GetBnrChecksum(Banner2);
        }
    } else {
        memset(Banner, 0, sizeof(Banner));
    }

    //store the crc for the game
    HLE_GetGameCRC(g_current_game_crc, (u8 *)Header, BannerCRC);

    //load up the data for the apploader
    SetFilePointer(FileHandle, 0x2440, 0, SEEK_SET);
    ReadFile(FileHandle, AppLoaderHeader, sizeof(AppLoaderHeader), &BytesRead, 0);

    if(DumpGCMBlockReads) {
        WriteFile(DumpFileHandle, AppLoaderHeader, sizeof(AppLoaderHeader), &BytesRead, 0);
    }

    //load the image
    SetFilePointer(FileHandle, 0x2460, 0, SEEK_SET);
    ReadFile(FileHandle, &Mem_RAM[0x81200000 & RAM_MASK], BSWAP32(AppLoaderHeader[5]), 
        &BytesRead, 0);

    if(DumpGCMBlockReads) {
        WriteFile(DumpFileHandle, &Mem_RAM[0x81200000 & RAM_MASK], BSWAP32(AppLoaderHeader[5]), 
            &BytesRead, 0);
    }

    //flip the memory around as needed
    for(i = 0; i < ((BytesRead >> 2) + 1); i++) {
        *(u32 *)(&Mem_RAM[(0x81200000 + (i * 4)) & RAM_MASK]) = 
            BSWAP32(*(u32 *)(&Mem_RAM[(0x81200000 + (i * 4)) & RAM_MASK]));
    }

    Bootrom(FSTInfo.MemLocation);
    Flipper_Open();
    Boot_AppLoader(AppLoaderHeader);
    HLE_ScanForPatches();

    return E_OK;
}

int ReadGCMInfo(char *filename, unsigned long *filesize, void *BannerBuffer /* 0x1960 bytes */, void *Header /* 0x440 bytes */)
{
    GCMFSTHeader	FSTInfo;
    DWORD			BytesRead;
    u32				TempData;
    u32				FileCount;
    u32				i;
    GCMFST *		GCMFSTData;
    HANDLE			GCMFileHandle;

    if(!BannerBuffer || !Header) {
        return E_ERR;
    }

    //open it up
    GCMFileHandle = CreateFile(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, NULL);
    if(GCMFileHandle == INVALID_HANDLE_VALUE) {
        return E_ERR;
    }

    //read the size
    *filesize = GetFileSize(GCMFileHandle, NULL);

    //read the header
    ReadFile(GCMFileHandle, Header, 0x440, &BytesRead, 0);

    //read the FST
    if(SetFilePointer(GCMFileHandle, 0x424, NULL, SEEK_SET) != 0x424) {
        CloseHandle(GCMFileHandle);
        return E_ERR;
    }

    //get the FST info header
    ReadFile(GCMFileHandle, &FSTInfo, sizeof(FSTInfo), &BytesRead, 0);
    if(BytesRead != sizeof(FSTInfo)) {
        CloseHandle(GCMFileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return E_ERR;
    }

    //swap the FST info
    FSTInfo.Offset = BSWAP32(FSTInfo.Offset) + 8;		//adjust for the 8 header bytes
    FSTInfo.Size = BSWAP32(FSTInfo.Size) - 8;			//adjust for the 8 header bytes
    FSTInfo.MaxSize = BSWAP32(FSTInfo.MaxSize) - 8;

    //seek to the position of the FST plus 8 bytes
    if(SetFilePointer(GCMFileHandle, FSTInfo.Offset, NULL, SEEK_SET) != FSTInfo.Offset) {
        CloseHandle(GCMFileHandle);
        return E_ERR;
    }

    //read 4 bytes for the number of files
    ReadFile(GCMFileHandle, &FileCount, 4, &BytesRead, 0);
    if(BytesRead != 4) {
        CloseHandle(GCMFileHandle);
        return E_ERR;
    }

    //allocate memory for the FST info and filenames
    //GCM's count the first 8 bytes + 4 byte file count as a file record. Remove it
    FileCount = BSWAP32(FileCount) - 1;
    GCMFSTData = (GCMFST *)malloc((FileCount+1) * sizeof(GCMFST));
    FileNames = (char *)malloc(FSTInfo.Size - (FileCount * sizeof(GCMFST)));
    memset(&GCMFSTData[FileCount], 0, sizeof(GCMFST));

    //read the data
    ReadFile(GCMFileHandle, GCMFSTData, FileCount * sizeof(GCMFST), &BytesRead, 0);
    if(BytesRead != (FileCount * sizeof(GCMFST))) {
        CloseHandle(GCMFileHandle);
        return E_ERR;
    }

    //get the filenames
    TempData = FSTInfo.Size - (FileCount * sizeof(GCMFST));
    ReadFile(GCMFileHandle, FileNames, TempData, &BytesRead, 0);

    //make all files uppercase
    for(i=0; i < TempData; i++) {
        if(FileNames[i] >= 'a' && FileNames[i] <= 'z')
            FileNames[i] &= 0xDF;
    }

    //seek thru the files for opening.bnr
    for(i=0; i < FileCount; i++) {
        if(strcmp(&FileNames[BSWAP32(GCMFSTData[i].NameOffset) & 0xFFFFFF], "OPENING.BNR") == 0) {
            //found the entry, read it's data and exit
            SetFilePointer(GCMFileHandle, BSWAP32(GCMFSTData[i].DiskAddr), NULL, SEEK_SET);
            ReadFile(GCMFileHandle, BannerBuffer, 0x1960, &BytesRead, 0);
            break;
        }
    }

    //close the file
    CloseHandle(GCMFileHandle);

    free(FileNames);
    free(GCMFSTData);

    //return if we found opening.bnr
    if (i >= FileCount) {
        return E_ERR;
    }
    return E_OK;
}

} // namespace
