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

#include <stdio.h>

#include "common.h"
#include "realdvd.h"
#include "powerpc/cpu_core.h"
#include "powerpc/cpu_core_regs.h"
#include "boot/bootrom.h"
#include "boot/apploader.h"
#include "hw/hw.h"
#include "hle/hle.h"
#include "gcm.h"
#include "memory.h"
#include "core.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

FILE*   g_file_handle = NULL;
FILE*   g_dump_file_handle = NULL;

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


void GCMFSTHeader::ToggleEndianness()
{
    Offset = BSWAP32(Offset);
    Size = BSWAP32(Size);
    MaxSize = BSWAP32(MaxSize);
    MemLocation = BSWAP32(MemLocation);
}

void GCMHeader::ToggleEndianness()
{
    // game code and maker code usually aren't printed swapped, so we shouldn't do that either..
    //game_code.hex = BSWAP32(game_code.hex);
    //maker_code = BSWAP16(maker_code);
    dvd_magic_word = BSWAP32(dvd_magic_word);
    debug_monitor_offset = BSWAP32(debug_monitor_offset);
    debug_monitor_load_addr = BSWAP32(debug_monitor_load_addr);
    main_dol_offset = BSWAP32(main_dol_offset);
    fst_header.ToggleEndianness();
    user_length = BSWAP32(user_length);
}

/// Gets the banner checksum
u8 GetBnrChecksum(void *banner)
{
    DVDBanner*  bnr  = (DVDBanner *)banner;
    DVDBanner2* bnr2 = (DVDBanner2*)banner;
    u8*         buf;
    u32         sum  = 0;

    LOG_NOTICE(TDVD, "GetBnrChecksum");

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
    GCMFileInfo *	GCMFilePtr;

    //LOG_NOTICE(TDVD, "GCMDVDRead");

    //if the file handle is invalid then exit
    if(g_file_handle == NULL)
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
    fseek(g_file_handle, GCMFilePtr->FileData->DiskAddr + GCMFilePtr->CurPos, SEEK_SET);

    //if the length puts the cursor past the end of the file, then adjust the length
    if((GCMFilePtr->CurPos + Len) > GCMFilePtr->FileData->FileSize)
        Len = GCMFilePtr->FileData->FileSize - GCMFilePtr->CurPos;

    //read from the file
    ReadLen = fread(MemPtr, 1, Len, g_file_handle);
    if(Len != ReadLen) {
        LOG_ERROR(TDVD, "Reading invalid area of file!\n");
        return 0;
    }

    if(DumpGCMBlockReads) {
        fwrite(MemPtr, 1, Len, g_dump_file_handle);
    }
    //adjust the current pointer
    GCMFilePtr->CurPos += ReadLen;
    return ReadLen;
}

DEFRealDVDSeek(GCMDVDSeek)
{
    GCMFileInfo *	GCMFilePtr;
    long			NewPos;

    //LOG_NOTICE(TDVD, "GCMDVDSeek");
    
    if(g_file_handle == NULL)
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
    fseek(g_file_handle, GCMFilePtr->FileData->DiskAddr + NewPos, SEEK_SET);
    NewPos = ftell(g_file_handle);

    //adjust the struct
    NewPos -= GCMFilePtr->FileData->DiskAddr;
    GCMFilePtr->CurPos = NewPos;
    return NewPos;
}

DEFRealDVDClose(GCMDVDClose)
{
    GCMFileInfo *		GCMFilePtr;
    GCMFileInfo *		OldGCMFilePtr;

    LOG_NOTICE(TDVD, "GCMDVDClose");

    if(g_file_handle == NULL) {
        return 0;
    }

    if(FilePtr == 0) {
        return 0;
    }

    //if the special id, update the file handle to the first entry
    if(FilePtr == REALDVD_LOWLEVEL) {
        //cleanup
        fclose(g_file_handle);

        if(DumpGCMBlockReads) {
            fclose(g_dump_file_handle);
        }

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

        g_file_handle = NULL;
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

    LOG_NOTICE(TDVD, "GCMDVDGetFileSize");

    if(g_file_handle == NULL)
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

    LOG_NOTICE(TDVD, "GCMDVDGetPos");

    if(g_file_handle == NULL)
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

    LOG_NOTICE(TDVD, "FindFSTEntry");

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

    LOG_NOTICE(TDVD, "ChangeDirEntry");

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

    LOG_NOTICE(TDVD, "GCMDVDChangeDir");

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

    LOG_NOTICE(TDVD, "GCMDVDOpen");

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
    return (uintptr_t)GCMFilePtr;
}

void ParseFSTTree(GCMFileData *FSTEntry, GCMFileData *RootFSTEntry, GCMFST **CurEntry, char *Filenames, u32 *Count, GCMFileData *ParentFST)
{
    u32				i;
    u32				OldCount;

   // LOG_NOTICE(TDVD, "ParseFSTTree");

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

/*
TODO(ShizZy): Make cross platform 2012-03-07
void DumpFSTEntry(HANDLE DumpHand, GCMFileData *Entry, u32 FileCount, u32 Level)
{
    u32		i;
    u32		x;
    char	FileBuff[512];

    LOG_NOTICE(TDVD, "DumpFSTEntry");

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

    LOG_NOTICE(TDVD, "DumpFST");

    DumpHand = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    DumpFSTEntry(DumpHand, Entry->FileList, Entry->FileCount, 0);
    CloseHandle(DumpHand);
}
*/

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
    char *			FileSlash;
    char			Header[SIZE_OF_GCM_HEADER];

    //if a file is already open, fail
    if(g_file_handle != NULL) {
        return E_ERR;
    }

    //open it up
    g_file_handle = fopen(filename, "rb");
    if (g_file_handle == NULL) {
        LOG_ERROR(TDVD, "Failed to open %s!", filename);
        return E_ERR;
    }

    if(DumpGCMBlockReads) {
        char dumpfilename[MAX_PATH];

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

        g_dump_file_handle = fopen(dumpfilename, "w+");
    }

    //setup the MSR
	ireg.MSR = MSR_BIT_DR | MSR_BIT_IR | MSR_BIT_FP;

    Memory_Open();

    //read the first 32 bytes into the root memory area
    BytesRead = fread(&Mem_RAM[0], 1, 32, g_file_handle);

    //get a copy of the CRC into the header
    memcpy(Header, &Mem_RAM[0], 32);

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, &Mem_RAM[0], 32, &BytesRead, 0);
    }

    //swap the memory around
    for(i = 0; i < (32 >> 2); i++) {
        *(u32 *)(&Mem_RAM[i * 4]) = BSWAP32(*(u32 *)(&Mem_RAM[i * 4]));
    }

    //read the game name, make sure the last byte is null terminated
    //0x400 - 0x20 = 3E0
    BytesRead = fread(g_current_game_name, 1, 0x3E0, g_file_handle);

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, g_current_game_name, 0x3E0, &BytesRead, 0);
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
    fseek(g_file_handle, 0x424, SEEK_SET);
    if (ftell(g_file_handle) != 0x424) {
        fclose(g_file_handle);
        g_file_handle = NULL;
        return E_ERR;
    }

    //get the FST info header
    BytesRead = fread(&FSTInfo, 1, sizeof(FSTInfo), g_file_handle);
    if(BytesRead != sizeof(FSTInfo))
    {
        fclose(g_file_handle);
        g_file_handle = NULL;
        return E_ERR;
    }

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, &FSTInfo, sizeof(FSTInfo), &BytesRead, 0);
    }
    //swap the FST info
    FSTInfo.Offset = BSWAP32(FSTInfo.Offset);
    FSTInfo.Size = BSWAP32(FSTInfo.Size);
    FSTInfo.MaxSize = BSWAP32(FSTInfo.MaxSize);
    FSTInfo.MemLocation = BSWAP32(FSTInfo.MemLocation) + RAM_24MB - (4*1024*1024);	//last 4 megs of mem

    Memory_Write32(0x80000038, FSTInfo.MemLocation);
    Memory_Write32(0x8000003C, FSTInfo.MaxSize);

    fseek(g_file_handle, FSTInfo.Offset + 8, SEEK_SET);
    if (ftell(g_file_handle) != (FSTInfo.Offset + 8)) {
        fclose(g_file_handle);
        g_file_handle = NULL;
        return E_ERR;
    }

    //read 4 bytes for the number of files
    BytesRead = fread(&FileCount, 1, 4, g_file_handle);
    if(BytesRead != 4)
    {
        fclose(g_file_handle);
        g_file_handle = NULL;
        return E_ERR;
    }

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, &FileCount, 4, &BytesRead, 0);
    }

    //go back. the first entry is empty but tells the number of files
    fseek(g_file_handle, FSTInfo.Offset, SEEK_SET);
    if (ftell(g_file_handle) != FSTInfo.Offset) {
        fclose(g_file_handle);
        g_file_handle = NULL;
        return E_ERR;
    }

    //allocate memory for the FST info and filenames
    FileCount = BSWAP32(FileCount);
    int foo = FileCount * sizeof(GCMFST);
    GCMFSTData = (GCMFST *) malloc((FileCount+1) * sizeof(GCMFST));
    FileNames = (char *)malloc(FSTInfo.Size - (foo));
    memset(&GCMFSTData[FileCount], 0, sizeof(GCMFST));

    //read the data
    BytesRead = fread(GCMFSTData, 1, foo, g_file_handle);
    if(BytesRead != (foo))
    {
        fclose(g_file_handle);
        g_file_handle = NULL;
        return E_ERR;
    }

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, GCMFSTData, FileCount * sizeof(GCMFST), &BytesRead, 0);
    }

    //make a copy of it to memory
    for(i = 0; i < (BytesRead >> 2); i++) {
        Memory_Write32(FSTInfo.MemLocation + (i * 4), *(u32 *)&(((u8 *)GCMFSTData)[i * 4]));
    }

    for(i = i * 4; i < BytesRead; i++) {
        Memory_Write8(FSTInfo.MemLocation + i, *(u32 *)&((u8 *)GCMFSTData)[i]);
    }

    //swap all of the numerical FST data in the GCM data
    for (i = 1; i < FileCount; i++) {
        GCMFSTData[i].DiskAddr = BSWAP32(GCMFSTData[i].DiskAddr);
        GCMFSTData[i].FileSize = BSWAP32(GCMFSTData[i].FileSize);
        GCMFSTData[i].NameOffset = BSWAP32(GCMFSTData[i].NameOffset);
    }

    TempData = FSTInfo.Size - (FileCount * sizeof(GCMFST));
    //ReadFile(FileHandle, FileNames, TempData, &BytesRead, 0);
    BytesRead = fread(FileNames, 1, TempData, g_file_handle);
    if(BytesRead != TempData)
    {
        fclose(g_file_handle);
        g_file_handle = NULL;
        free(FileNames);
        free(GCMFSTData);
        return E_ERR;
    }

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, FileNames, TempData, &BytesRead, 0);
    }

    //copy the filenames
    x = FSTInfo.MemLocation + (FileCount * sizeof(GCMFST));
    for(i = 0; i < (TempData >> 2); i++) {
        Memory_Write32(x + (i * 4), *(u32 *)&FileNames[i * 4]);
    }

    for(i = (i * 4); i < FileCount; i++) {
        Memory_Write8(x + i, *(u32 *)&FileNames[i]);
    }

    LOG_NOTICE(TDVD, "FST Data loaded 0x%08X bytes to 0x%08X. File names @ 0x%08X\n", 
        FSTInfo.Size, FSTInfo.MemLocation, FSTInfo.MemLocation + (FileCount * sizeof(GCMFST)));

    //make all files uppercase
    for (i=0; i < TempData; i++) {
        if (FileNames[i] >= 'a' && FileNames[i] <= 'z') {
            FileNames[i] &= 0xDF;
        }
    }

    FST = (GCMFileData *)malloc(sizeof(GCMFileData));
    if (!FST) {
        fclose(g_file_handle);
        g_file_handle = NULL;
        free(FileNames);
        free(GCMFSTData);
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
    size_t pos = ftell(g_file_handle);
    fseek(g_file_handle, 0L, SEEK_END);
    FST->FileSize = ftell(g_file_handle);
    fseek(g_file_handle, pos, SEEK_SET);
    FST->DiskAddr = 0;
    FST->Filename = NULL;
    FST->IsDirectory = 1;

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, &FST->FileSize, sizeof(FST->FileSize), &BytesRead, 0);
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
        fseek(g_file_handle, BannerData->DiskAddr, SEEK_SET);
        if (BannerData->FileSize == sizeof(Banner)) {
            BytesRead = fread(Banner, BannerData->FileSize, 1, g_file_handle);

            if(DumpGCMBlockReads) {
// TODO
//                WriteFile(DumpFileHandle, Banner, BannerData->FileSize, &BytesRead, 0);
            }

            BannerCRC = GetBnrChecksum(Banner);
        } else if (BannerData->FileSize > sizeof(Banner) && (BannerData->FileSize - 0x1820) % 0x140 == 0x00) {
            //ReadFile(FileHandle, Banner2, BannerData->FileSize, &BytesRead, 0);
            BytesRead = fread(Banner2, BannerData->FileSize, 1, g_file_handle);

            if(DumpGCMBlockReads) {
// TODO
//                WriteFile(DumpFileHandle, Banner2, BannerData->FileSize, &BytesRead, 0);
            }

            BannerCRC = GetBnrChecksum(Banner2);
        }
    } else {
        memset(Banner, 0, sizeof(Banner));
    }

    //store the crc for the game
    HLE_GetGameCRC(g_current_game_crc, (u8 *)Header, BannerCRC);

    //load up the data for the apploader
    fseek(g_file_handle, 0x2440, SEEK_SET);
    BytesRead = fread(AppLoaderHeader, sizeof(AppLoaderHeader), 1, g_file_handle);

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, AppLoaderHeader, sizeof(AppLoaderHeader), &BytesRead, 0);
    }

    //load the image
    fseek(g_file_handle, 0x2460, SEEK_SET);
    BytesRead = fread(&Mem_RAM[0x81200000 & RAM_MASK], 1, BSWAP32(AppLoaderHeader[5]), g_file_handle);

    if(DumpGCMBlockReads) {
// TODO
//        WriteFile(DumpFileHandle, &Mem_RAM[0x81200000 & RAM_MASK], BSWAP32(AppLoaderHeader[5]), 
//            &BytesRead, 0);
    }

    //flip the memory around as needed
    for (i = 0; i < ((BytesRead >> 2) + 1); i++) {
        *(u32 *)(&Mem_RAM[(0x81200000 + (i * 4)) & RAM_MASK]) = 
            BSWAP32(*(u32 *)(&Mem_RAM[(0x81200000 + (i * 4)) & RAM_MASK]));
    }

    Bootrom(FSTInfo.MemLocation);
    Flipper_Open(); // TODO: wrong place for this!!
    Boot_AppLoader(AppLoaderHeader);
    HLE_ScanForPatches();

    return E_OK;
}

int ReadGCMInfo(const char *filename, unsigned long *filesize, void *BannerBuffer, GCMHeader *Header)
{
    u32      read_count;
    u32      gcm_file_count;
    u32      i;
    u32      x;
    GCMFST*  gcm_fst_data = NULL;
    char*    file_names = NULL;
    int      file_names_size;
    int      foo; // gotta admire ShizZy's creativity when naming variables
    FILE*    file_handle;
    u8*      header_data[0x1000];
    int ret = E_ERR;

    // Read to stack if caller doesn't request header
    GCMHeader gcm_header;
    if (!Header)
        Header = &gcm_header;

    // Open file
    file_handle = fopen(filename, "rb");
    if (file_handle == NULL) {
        return E_ERR;
    }

    if (filesize)
        *filesize = common::FileSize(file_handle);

    // Read the GCM header, check magic word
    read_count = fread(Header, 1, sizeof(GCMHeader), file_handle);
    if (read_count != sizeof(GCMHeader))
        goto cleanup;

    Header->ToggleEndianness();
    if (((GCMHeader*)Header)->dvd_magic_word != GCM_HEADER_MAGIC_WORD)
        goto cleanup;

    // TODO(neobrain): Is this correct?
    Header->fst_header.MemLocation += RAM_24MB - 4*1024*1024; //last 4 megs of mem

    fseek(file_handle, Header->fst_header.Offset + 8, SEEK_SET);
    if (ftell(file_handle) != (Header->fst_header.Offset + 8))
        goto cleanup;

    // Read 4 bytes for the number of files
    read_count = fread(&gcm_file_count, 1, 4, file_handle);
    if (read_count != 4)
        goto cleanup;

    // Go back. the first entry is empty but tells the number of files
    fseek(file_handle, Header->fst_header.Offset, SEEK_SET);
    if (ftell(file_handle) != Header->fst_header.Offset)
        goto cleanup;

    // Allocate memory for the FST info and filenames
    gcm_file_count = BSWAP32(gcm_file_count);
    gcm_fst_data = new GCMFST[gcm_file_count+1];

    foo = gcm_file_count * sizeof(GCMFST);
    file_names = new char[Header->fst_header.Size - foo];

    // Read the data
    read_count = fread(gcm_fst_data, 1, foo, file_handle);
    if (read_count != foo)
        goto cleanup;

    // Swap all of the numerical FST data in the GCM data
    for (i = 1; i < gcm_file_count; i++) {
        gcm_fst_data[i].DiskAddr = BSWAP32(gcm_fst_data[i].DiskAddr);
        gcm_fst_data[i].FileSize = BSWAP32(gcm_fst_data[i].FileSize);
        gcm_fst_data[i].NameOffset = BSWAP32(gcm_fst_data[i].NameOffset);
    }
    file_names_size = Header->fst_header.Size - (gcm_file_count * sizeof(GCMFST));
    read_count = fread(file_names, 1, file_names_size, file_handle);
    if (read_count != file_names_size)
        goto cleanup;

    // Seek thru the files for opening.bnr
    for (i=0; i < gcm_file_count; i++) {
        common::UpperStr(&file_names[(gcm_fst_data[i].NameOffset) & 0xFFFFFF]);
        if (strcmp(&file_names[(gcm_fst_data[i].NameOffset) & 0xFFFFFF], "OPENING.BNR") == 0) {
            // Found the entry, read it's data and exit
            if (BannerBuffer)
            {
                fseek(file_handle, (gcm_fst_data[i].DiskAddr), SEEK_SET);
                read_count = fread(BannerBuffer, 0x1960, 1, file_handle);
            }
            break;
        }
    }

    // Only succeed if we found opening.bnr
    if (i < gcm_file_count)
        ret = E_OK;

cleanup:
    delete[] file_names;
    delete[] gcm_fst_data;
    fclose(file_handle);

    return ret;
}

} // namespace
