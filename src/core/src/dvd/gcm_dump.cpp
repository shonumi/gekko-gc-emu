/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    gcm_dump.cpp
 * \author  Lightning
 * \date    2006-01-01
 * \brief   Interface for loading a dump of a GCM GameCube DVD image
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
#include "boot/bootrom.h"
#include "boot/apploader.h"
#include "gcm.h"
#include "memory.h"
#include "hle/hle.h"
#include "powerpc/cpu_core.h"
#include "hw/hw.h"
#include "loader.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

HANDLE		DmpFileHandle = INVALID_HANDLE_VALUE;

u8 GetBnrChecksum(void *banner);

DEFRealDVDRead(GCMDMPDVDRead)
{
	DWORD			ReadLen;
	GCMFileInfo *	GCMFilePtr;

	//if the file handle is invalid then exit
	if(DmpFileHandle == INVALID_HANDLE_VALUE)
		return 0;

	if(FilePtr == 0)
		return 0;

	if(!Len)
		return 0;

	if(GetFileSize(DmpFileHandle, 0) <= SetFilePointer(DmpFileHandle, 0, 0, SEEK_CUR))
	{
		cpu->pause = 1;
		LOG_NOTICE(TDVD, "----------------------------------------------------------\n");
		LOG_NOTICE(TDVD, "No more data to read from dump file - dump new file please\n");
		LOG_NOTICE(TDVD, "----------------------------------------------------------\n");
		return 0;
	}

	//if the special id, update the file handle to the first entry
	if(FilePtr == REALDVD_LOWLEVEL)
		GCMFilePtr = LowLevelPtr;
	else
		GCMFilePtr = (GCMFileInfo *)FilePtr;

	if(GCMFilePtr->ID != GCMFILEID)
		return 0;

	//if the length puts the cursor past the end of the file, then adjust the length
	if((GCMFilePtr->CurPos + Len) > GCMFilePtr->FileData->FileSize)
		Len = GCMFilePtr->FileData->FileSize - GCMFilePtr->CurPos;

	//read from the file
	if(!ReadFile(DmpFileHandle, MemPtr, Len, &ReadLen, 0))
		return 0;

	//adjust the current pointer
	GCMFilePtr->CurPos += ReadLen;
	return ReadLen;
}

DEFRealDVDSeek(GCMDMPDVDSeek)
{
	GCMFileInfo *	GCMFilePtr;
	long			NewPos;

	if(DmpFileHandle == INVALID_HANDLE_VALUE)
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

	GCMFilePtr->CurPos = NewPos;
	return NewPos;
}

DEFRealDVDClose(GCMDMPDVDClose)
{
	GCMFileInfo *		GCMFilePtr;
	GCMFileInfo *		OldGCMFilePtr;

	if(DmpFileHandle == INVALID_HANDLE_VALUE)
		return 0;

	if(FilePtr == 0)
		return 0;

	//if the special id, update the file handle to the first entry
	if(FilePtr == REALDVD_LOWLEVEL)
	{
		//cleanup
		CloseHandle(DmpFileHandle);

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
		FST = NULL;
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

DEFRealDVDGetFileSize(GCMDMPDVDGetFileSize)
{
	GCMFileInfo *		GCMFilePtr;

	if(DmpFileHandle == INVALID_HANDLE_VALUE)
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

DEFRealDVDGetPos(GCMDMPDVDGetPos)
{
	GCMFileInfo *		GCMFilePtr;

	if(DmpFileHandle == INVALID_HANDLE_VALUE)
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

DEFRealDVDChangeDir(GCMDMPDVDChangeDir)
{
	char            NewDir[256];
	u32             i;
	GCMFileData*    NewPath;

	//copy the directory passed in
	memset(NewDir, 0, sizeof(NewDir));
	strcpy(NewDir, ChangeDirPath);

	//go thru the directory and change / to .
	for (i=0; i < strlen(ChangeDirPath); i++) {
		if (NewDir[i] == '/') {
            NewDir[i] = 0x00;
        }
    }

	NewPath = ChangeDirEntry(GCMCurDir, NewDir);
	if (NewPath) {
		GCMCurDir = NewPath;
    }
	return (NewPath != NULL);
}

DEFRealDVDOpen(GCMDMPDVDOpen)
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

int LoadDMP(char *filename) {
	GCMFSTHeader	FSTInfo;
	DWORD			BytesRead;
	u32				TempData;
	u32				FileCount;
	u32				i;
	u32				x;
	GCMFST *		GCMFSTData;
	GCMFST *		CurGCMFSTData;
	u32				AppLoaderHeader[8];
	u8				Banner[0x1960];
	char			BannerFilename[] = "OPENING.BNR\0\0";
	u8				BannerCRC;
	GCMFileData *	BannerData;
	char			Header[SIZE_OF_GCM_HEADER];

	//if a file is already open, fail
	if(DmpFileHandle != INVALID_HANDLE_VALUE)
		return E_ERR;

	//open it up
	DmpFileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
	if(DmpFileHandle == INVALID_HANDLE_VALUE)
		return E_ERR;

	//setup the MSR
	set_ireg_MSR(MSR_BIT_DR | MSR_BIT_IR | MSR_BIT_FP);

	Memory_Open();

	//setup the cpu
	for(i=0; i < 16; i++)
		set_ireg_sr(i, 0x80000000);
    set_ireg_spr(I_IBAT0U, 0x80001fff);
	set_ireg_spr(I_IBAT0L, 0x00000002);
    set_ireg_spr(I_IBAT1U, 0xc0001fff);
	set_ireg_spr(I_IBAT1L, 0x0000002a);
	set_ireg_spr(I_DBAT0U, 0x80001fff);
	set_ireg_spr(I_DBAT0L, 0x00000002);
    set_ireg_spr(I_DBAT1U, 0xc0001fff);
	set_ireg_spr(I_DBAT1L, 0x0000002a);
	set_ireg_spr(287, 0x00083214);


	//setup the memory for the dolphin os
    Memory_Write32(0x800000FC, GEKKO_CLOCK);
    Memory_Write32(0x800000F8, GEKKO_CLOCK / 3);
	Memory_Write32(0x80000028, RAM_24MB);
	Memory_Write32(0x800000F0, RAM_24MB);
	Memory_Write32(0x8000002C, 1);
	Memory_Write32(0x80000034, 0x817FE8C0);
	Memory_Write32(0x80000038, 0x817FE8C0);
	Memory_Write32(0x8000003C, 0x00000024);

	//setup a dummy system handler, rfi
    Memory_Write32(0x80000C00, 0x6400004c);

	//read the first 32 bytes into the root memory area
	ReadFile(DmpFileHandle, (u32 *)(&Mem_RAM[0x80000000 & RAM_MASK]), 32, &BytesRead, 0);

	//get a copy of the CRC into the header
	memcpy(Header, &Mem_RAM[0], 32);

	for(i = 0; i < 32; i+=4)
		*(u32 *)(&Mem_RAM[i]) = BSWAP32(*(u32 *)(&Mem_RAM[i]));

	//read the game name, make sure the last byte is null terminated
	//0x400 - 0x20 = 3E0
    ReadFile(DmpFileHandle, dvd::g_current_game_name, 0x3E0, &BytesRead, 0);

	LOG_NOTICE(TDVD, "Loading %s\n", dvd::g_current_game_name);

	//see if we are in pal mode
	//was 0x80000003
	if(Memory_Read8(0x80000003) == (u8)'P') Memory_Write32(0x800000CC, 1);

	//get the FST info header
	ReadFile(DmpFileHandle, &FSTInfo, sizeof(FSTInfo), &BytesRead, 0);
	if(BytesRead != sizeof(FSTInfo))
	{
		CloseHandle(DmpFileHandle);
		DmpFileHandle = INVALID_HANDLE_VALUE;
		return E_ERR;
	}

	//swap the FST info
	FSTInfo.Offset = BSWAP32(FSTInfo.Offset);
	FSTInfo.Size = BSWAP32(FSTInfo.Size);
	FSTInfo.MaxSize = BSWAP32(FSTInfo.MaxSize);
	FSTInfo.MemLocation = BSWAP32(FSTInfo.MemLocation) + RAM_24MB - (4*1024*1024);	//last 4 megs of mem

	Memory_Write32(0x80000038, FSTInfo.MemLocation);
	Memory_Write32(0x8000003C, FSTInfo.MaxSize);
	

	//read 4 bytes for the number of files
	ReadFile(DmpFileHandle, &FileCount, 4, &BytesRead, 0);
	if(BytesRead != 4)
	{
		CloseHandle(DmpFileHandle);
		DmpFileHandle = INVALID_HANDLE_VALUE;
		return E_ERR;
	}

	//allocate memory for the FST info and filenames
	FileCount = BSWAP32(FileCount);
	GCMFSTData = (GCMFST *)malloc((FileCount+1) * sizeof(GCMFST));
	FileNames = (char *)malloc(FSTInfo.Size - (FileCount * sizeof(GCMFST)));
	memset(&GCMFSTData[FileCount], 0, sizeof(GCMFST));

	LOG_NOTICE(TDVD, "FST Data loaded 0x%08X bytes to 0x%08X. File names @ 0x%08X\n", FSTInfo.Size, FSTInfo.MemLocation, FSTInfo.MemLocation + (FileCount * sizeof(GCMFST)));

	//read the data
	ReadFile(DmpFileHandle, GCMFSTData, FileCount * sizeof(GCMFST), &BytesRead, 0);
	if(BytesRead != (FileCount * sizeof(GCMFST)))
	{
		CloseHandle(DmpFileHandle);
		DmpFileHandle = INVALID_HANDLE_VALUE;
		return E_ERR;
	}

	//make a copy of it to memory
	for(i = 0; i < (BytesRead >> 2); i++)
		Memory_Write32(FSTInfo.MemLocation + (i * 4), *(u32 *)&(((u8 *)GCMFSTData)[i * 4]));

	for(i = i * 4; i < BytesRead; i++)
		Memory_Write8(FSTInfo.MemLocation + i, *(u32 *)&((u8 *)GCMFSTData)[i]);

	//swap all of the FST data
	for(i=1; i < FileCount; i++)
	{
		GCMFSTData[i].DiskAddr = BSWAP32(GCMFSTData[i].DiskAddr);
		GCMFSTData[i].FileSize = BSWAP32(GCMFSTData[i].FileSize);
		GCMFSTData[i].NameOffset = BSWAP32(GCMFSTData[i].NameOffset);
	}

	TempData = FSTInfo.Size - (FileCount * sizeof(GCMFST));
	ReadFile(DmpFileHandle, FileNames, TempData, &BytesRead, 0);
	if(BytesRead != TempData)
	{
		CloseHandle(DmpFileHandle);
		free(FileNames);
		free(GCMFSTData);
		DmpFileHandle = INVALID_HANDLE_VALUE;
		return E_ERR;
	}

	//copy the filenames
	x = FSTInfo.MemLocation + (FileCount * sizeof(GCMFST));
	for(i = 0; i < (TempData >> 2); i++)
		Memory_Write32(x + (i * 4), *(u32 *)&FileNames[i * 4]);

	for(i = (i * 4); i < FileCount; i++)
		Memory_Write8(x + i, *(u32 *)&FileNames[i]);

//	memcpy(MEMPTR32(FSTInfo.MemLocation + (FileCount * sizeof(GCMFST))), FileNames, TempData);

	//make all files uppercase
	for(i=0; i < TempData; i++)
	{
		if(FileNames[i] >= 'a' && FileNames[i] <= 'z')
			FileNames[i] &= 0xDF;
	}

	FST = (GCMFileData *)malloc(sizeof(GCMFileData));
	if(!FST)
	{
		CloseHandle(DmpFileHandle);
		free(FileNames);
		free(GCMFSTData);
		DmpFileHandle = INVALID_HANDLE_VALUE;
		return E_ERR;
	}

	memset(FST, 0, sizeof(GCMFileData));

	if(FileCount != 0)
		FileCount--;

	//setup the root node
	FST->FileCount = FileCount;
	FST->FileList = (GCMFileData *)malloc(FileCount * sizeof(GCMFileData));
	memset(FST->FileList, 0, FileCount * sizeof(GCMFileData));
	FST->DiskAddr = 0;
	FST->Filename = NULL;
	FST->IsDirectory = 1;

	ReadFile(DmpFileHandle, &FST->FileSize, sizeof(FST->FileSize), &BytesRead, 0);

	//setup the file count
	i = 0;
	FST->FileCount = AdjustFSTCounts(&GCMFSTData[1], &i, FileCount);

	//now, parse up the FST info into the tree
	TempData = FST->FileCount;
	CurGCMFSTData = &GCMFSTData[1];
	FileCount=0;
	for(i=0; i < FST->FileCount; i++)
	{
		ParseFSTTree(&FST->FileList[i], FST->FileList, &CurGCMFSTData, FileNames, &TempData, FST);
		CurGCMFSTData++;
	}

	//set the current directory to the root
	GCMCurDir = FST;

	//DumpFST("c:\\temp\\fstdat.txt", FST);

	//setup the root file entry
	LowLevelPtr = (GCMFileInfo *)malloc(sizeof(GCMFileInfo));
	memset(LowLevelPtr, 0, sizeof(GCMFileInfo));
	LowLevelPtr->ID = GCMFILEID;
	LowLevelPtr->FileData = FST;
	LowLevelPtr->PrevPtr = NULL;
	LowLevelPtr->NextPtr = NULL;

	//setup the RealDVD entries
	RealDVDOpen = &GCMDMPDVDOpen;
	RealDVDSeek = &GCMDMPDVDSeek;
	RealDVDRead = &GCMDMPDVDRead;
	RealDVDGetFileSize = &GCMDMPDVDGetFileSize;
	RealDVDClose = &GCMDMPDVDClose;
	RealDVDGetPos = &GCMDMPDVDGetPos;
	RealDVDChangeDir = &GCMDMPDVDChangeDir;

	//find the opening.bnr
	BannerData = FindFSTEntry(FST, BannerFilename);
	if(BannerData && (BannerData->FileSize == sizeof(Banner)))
	{
		//read the banner
		ReadFile(DmpFileHandle, Banner, sizeof(Banner), &BytesRead, 0);
	}
	else
		memset(Banner, 0, sizeof(Banner));

	BannerCRC = GetBnrChecksum(Banner);

	//store the crc for the game
    HLE_GetGameCRC(dvd::g_current_game_crc, (u8 *)Header, BannerCRC);

	//load up the data for the apploader
	ReadFile(DmpFileHandle, AppLoaderHeader, sizeof(AppLoaderHeader), &BytesRead, 0);
	
	//load the image
	ReadFile(DmpFileHandle, &Mem_RAM[0x81200000 & RAM_MASK], BSWAP32(AppLoaderHeader[5]), &BytesRead, 0);

	//flip the memory around as needed
	for(i = 0; i < ((BytesRead >> 2) + 1); i++)
	{
		*(u32 *)(&Mem_RAM[(0x81200000 + (i * 4)) & RAM_MASK]) = 
			BSWAP32(*(u32 *)(&Mem_RAM[(0x81200000 + (i * 4)) & RAM_MASK]));
	}

	Flipper_Open();
	Boot_AppLoader(AppLoaderHeader);
	HLE_ScanForPatches();

	return E_OK;
}

} // namespace
