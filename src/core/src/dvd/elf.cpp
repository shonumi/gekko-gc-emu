/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    elf.cpp
 * @author  Lightning, ShizZy
 * @date    2006-01-01
 * @brief   Interface for loading an ELF
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
#include "platform.h"
#if EMU_PLATFORM == PLATFORM_LINUX
#include <unistd.h>
#endif
#include "memory.h"
#include "hw/hw.h"
#include "powerpc/cpu_core.h"
#include "hle/hle.h"
#include "boot/bootrom.h"

#include "realdvd.h"
#include "gcm.h"
#include "loader.h"
#include "elf.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

bool bElfLoaded=false;

u8 shstrtab[0x800];
u8 symtab[0x80000];
u8 strtab[0x80000];

u32 symindex=0;
Elf32_Sym * pSymTab = (Elf32_Sym *)symtab;
char ELFCurrentDir[256];

char DVDRootPath[1024];

typedef struct {
    char *	Filename;
    u32		FileSize;
    u32		FileOffset;
} ELFFileInfo;

ELFFileInfo	*ELFFiles;
u32			CurFilePos;

DEFRealDVDRead(ELFDVDRead)
{
    ELFFileInfo	*CurFile;
    FILE		*DataFilePtr;
    u32			Ret;

    if(FilePtr == REALDVD_LOWLEVEL)
    {
        CurFile = ELFFiles;
        while(CurFile->FileOffset)
        {
            if((CurFilePos >= CurFile->FileOffset) && (CurFilePos < (CurFile->FileOffset + CurFile->FileSize)))
            {
                //found our file, open it and read it
                DataFilePtr = fopen(CurFile->Filename, "r");
                if(DataFilePtr != 0)
                {
                    //got a file, seek to a spot and read it
                    fseek(DataFilePtr, CurFilePos - CurFile->FileOffset, SEEK_SET);
                    Ret = fread(MemPtr, Len, 1, DataFilePtr);
                    fclose(DataFilePtr);

                    //adjust the length if at the end of the file
                    if(Len > (CurFile->FileSize - (CurFilePos - CurFile->FileOffset)))
                        Len = CurFile->FileSize - (CurFilePos - CurFile->FileOffset);

                    CurFilePos += Len;
                    if(Ret)
                        Ret = Len;

                    return Ret;
                }
            }

            //move on to the next file
            CurFile++;
        }

        return Len;
    }

    //if successful then return the length
    if(fread(MemPtr, Len, 1, (FILE *)FilePtr))
        return Len;

    return 0;
}

DEFRealDVDSeek(ELFDVDSeek)
{
    int fSeekType;

    if(FilePtr == REALDVD_LOWLEVEL)
    {
        //return where we were suppose to move to
        switch(SeekType)
        {
        case REALDVDSEEK_START:
            CurFilePos = SeekPos;
            break;
        case REALDVDSEEK_CUR:
            CurFilePos += SeekPos;
            break;
        case REALDVDSEEK_END:
            CurFilePos = (1024*1024*1024*1.4) - SeekPos;
            break;
        };

        return CurFilePos;
    }

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

DEFRealDVDOpen(ELFDVDOpen)
{
    char FileNameBuff[256];
    char TempDir[256];
    int i = 0;
    u32	NewHandle;

    //adjust the filename if a / is at the beginning
    if(filename[0] == '/') i=1;
    sprintf(FileNameBuff, "%s", &filename[i]);

    //convert the / to \'s
    for(i=0;i<strlen(FileNameBuff);i++)
    {
        if(FileNameBuff[i]=='/') FileNameBuff[i]='\\';
    }

    //change the directory
    getcwd(TempDir, 256);
    chdir(ELFCurrentDir);

    //open up the file
    NewHandle = (u8*)fopen(FileNameBuff, "rb") - (u8*)NULL;

    //put the directory back
    chdir(TempDir);
    return NewHandle;
}

DEFRealDVDGetFileSize(ELFDVDGetFileSize)
{
    u32		LastPos;
    u32		FileSize;
    LastPos = ftell((FILE *)FilePtr);
    fseek((FILE *)FilePtr, 0, SEEK_END);
    FileSize = ftell((FILE *)FilePtr);
    fseek((FILE *)FilePtr, LastPos, SEEK_SET);
    return FileSize;
}

DEFRealDVDClose(ELFDVDClose)
{
    ELFFileInfo	*CurFile;

    if(FilePtr != REALDVD_LOWLEVEL)
        fclose((FILE *)FilePtr);
    else
    {
        //cleanup
        CurFile = ELFFiles;
        while(CurFile->FileOffset != 0)
        {
            //if we have a filename, free it
            if(CurFile->Filename)
                free(CurFile->Filename);

            //next index
            CurFile++;
        };

        free(ELFFiles);
        ELFFiles = 0;

        //reset the dvd access
        ResetRealDVD();
    }

    return 1;
}

DEFRealDVDGetPos(ELFDVDGetPos)
{
    return ftell((FILE *)FilePtr);
}

DEFRealDVDChangeDir(ELFDVDChangeDir)
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
    chdir(ELFCurrentDir);
    i = chdir(NewDir);

    //if no error, then save it
    if(!i) getcwd(ELFCurrentDir, 256);

    //swap the directory back for the app
    chdir(TempCurDir);
    return i;
}

bool ELF_LoadSection(FILE* f, u32 srcaddr, u32 dstaddr, u32 len)
{
    u32	x;

    if(!f)
    {
        LOG_ERROR(TDVD, "Invalid Loaded ELF Has No Size!");
        return false;
    }

    len = BSWAP32(len);
    fseek(f,BSWAP32(srcaddr),SEEK_SET);
    dstaddr = BSWAP32(dstaddr) & RAM_MASK;
    fread(&Mem_RAM[dstaddr], len, 1, f);

    for(x = dstaddr; x < (dstaddr + len); x+=4)
        *(u32 *)&Mem_RAM[x] = BSWAP32(*(u32 *)&Mem_RAM[x]);

    LOG_NOTICE(TDVD, "ELF %08X bytes copied to address %08X", BSWAP32(len), BSWAP32(dstaddr));

    return true;
}
#if EMU_PLATFORM == PLATFORM_WINDOWS
/*
u32 ELF_CountFilesWindows(char *CurPath, u32 *FileIndex, u32 *FileNameIndex)
{
    WIN32_FIND_DATA	FileData;
    HANDLE			FindHandle;
    char			NewDir[MAX_PATH];
    u32				TotalSize = 0;

    strcpy(NewDir, CurPath);
    strcat(NewDir, "\\*");

    //start looping and creating file entries in memory
    FindHandle = FindFirstFile(NewDir, &FileData);
    if(FindHandle)
    {
        //found a ., need to get past .. then we are on a normal file
        FindNextFile(FindHandle, &FileData);

        //now get a normal file
        while(FindNextFile(FindHandle, &FileData))
        {
            (*FileNameIndex) += strlen(FileData.cFileName) + 1;

            //if a directory then process it
            if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                strcpy(NewDir, CurPath);
                strcat(NewDir, "\\");
                strcat(NewDir, FileData.cFileName);

                //directory, set the size of the file to the number of entries
                (*FileIndex)++;
                TotalSize += ELF_CountFilesWindows(NewDir, FileIndex, FileNameIndex);
            }
            else
            {
                //count the file
                (*FileIndex)++;
                TotalSize += FileData.nFileSizeLow;
            }
        };

        //close our search
        FindClose(FindHandle);
    }

    return TotalSize;
}

u32 ELF_CreateFileStructureWindows(char *CurPath, GCMFST *FileEntries, u32 *FileIndex, char *FileNames, u32 *FileNameIndex, u32 *FilePos)
{
    WIN32_FIND_DATA	FileData;
    HANDLE			FindHandle;
    char			NewDir[MAX_PATH];
    u32				FileCount;
    u32				OldIndex;		
    u32				ParentDir;

    ParentDir = *FileIndex;
    FileCount = 0;

    strcpy(NewDir, CurPath);
    strcat(NewDir, "\\*");

    //start looping and creating file entries in memory
    FindHandle = FindFirstFile(NewDir, &FileData);
    if(FindHandle)
    {
        //found a ., need to get past .. then we are on a normal file
        FindNextFile(FindHandle, &FileData);

        //now get a normal file
        while(FindNextFile(FindHandle, &FileData))
        {
            //setup the filename info
            FileEntries[*FileIndex].NameOffset = *FileNameIndex;
            strcpy(&FileNames[*FileNameIndex], FileData.cFileName);
            (*FileNameIndex) += strlen(FileData.cFileName) + 1;

            if(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                strcpy(NewDir, CurPath);
                strcat(NewDir, "\\");
                strcat(NewDir, FileData.cFileName);

                //update our name offset to flag as a directory
                FileEntries[*FileIndex].NameOffset |= 0x01000000;
                FileEntries[*FileIndex].DiskAddr = ParentDir;

                //setup a basic entry being this is a directory
                ELFFiles[*FileIndex].FileOffset = 1;

                //directory, set the size of the file to the number of entries
                OldIndex = *FileIndex;
                (*FileIndex)++;

                ELF_CreateFileStructureWindows(NewDir, FileEntries, FileIndex, FileNames, FileNameIndex, FilePos);
                FileEntries[OldIndex].FileSize = *FileIndex + 1;
            }
            else
            {
                //set the file size
                ELFFiles[*FileIndex].FileOffset = *FilePos;
                ELFFiles[*FileIndex].FileSize = FileData.nFileSizeLow;
                ELFFiles[*FileIndex].Filename = (char *)malloc(strlen(FileData.cFileName) + strlen(CurPath) + 2);

                //create the full path and filename
                strcpy(ELFFiles[*FileIndex].Filename, CurPath);
                strcat(ELFFiles[*FileIndex].Filename, "\\");
                strcat(ELFFiles[*FileIndex].Filename, FileData.cFileName);

                FileEntries[*FileIndex].FileSize = FileData.nFileSizeLow;
                FileEntries[*FileIndex].DiskAddr = *FilePos;

                //change the file data position
                (*FilePos) += FileData.nFileSizeLow;

                //move to the next index
                (*FileIndex)++;
                FileCount++;
            }
        };

        //close our search
        FindClose(FindHandle);
    }

    return FileCount;
}
*/
#endif

u32 ELF_CreateFileStructure(char *ELFFileName)
{
    GCMFST	*FSTData;
    char	FilePath[MAX_PATH];
    char	*LastSlash;
    u32		FileCount;
    u32		FileNameLen;
    GCMFST	*FileInfo;
    char	*FileNames;
    u32		DataSize;
    u32		DataPos;
    u32		*MemSwap;
    u32		TotalFileSize;

    //go thru the dvd root directory and create file entries in memory

    //if we have a path then use it, otherwise assume local
    // TODO: Crashes if we pass "sample.elf" instead of "./sample.elf"
    if(DVDRootPath[0]) {
        strcpy(FilePath, DVDRootPath);
    } else {
        //add dvdroot to the end of the path
        strcpy(FilePath, ELFFileName);
        LastSlash = strrchr(FilePath, '\\');
        if(LastSlash) {
            *LastSlash = 0;
            strcat(FilePath, "\\dvdroot");
        } else {
            LastSlash = strrchr(FilePath, '/');
            *LastSlash = 0;
            strcat(FilePath, "/dvdroot");
        }
    }

    //setup the current working directory
    strcpy(ELFCurrentDir, FilePath);

    //go thru and find directories for the system	
    FileCount = 0;
    FileNameLen = 0;
/*
#if EMU_PLATFORM == PLATFORM_WINDOWS
    TotalFileSize = ELF_CountFilesWindows(FilePath, &FileCount, &FileNameLen);

    //verify our total file size is less than 1.39gb (leaving room for headers/etc on a dvd image)
    if(TotalFileSize > (1024*1024*1024*1.39))
    {
        LOG_ERROR(TDVD, "Error: dvdroot folder is %d bytes larger than 1.39GB. File system not setup!\n", TotalFileSize - (1024*1024*1024*1.39));
        return 0;
    }
    else
    {
        //set the start position of data at 10mb
        TotalFileSize = 10*1024*1024;
    }
#else*/
    TotalFileSize = 10*1024*1024;
//#endif

    //setup our memory positions for the data, need to count a root entry
    DataSize = (sizeof(GCMFST) * (FileCount+1));
    DataPos = RAM_24MB - DataSize - ((FileNameLen & ~3) + 4);

    LOG_NOTICE(TDVD, "ELF File Data: Loading %d files to 0x%08X, Filenames at 0x%08X", FileCount,
        0x80000000 + DataPos, 0x80000000 + DataPos + DataSize);

    FileInfo = (GCMFST *)(&Mem_RAM[DataPos + sizeof(GCMFST)]);
    FileNames = (char *)(&Mem_RAM[DataPos + DataSize]);

    //put the file data at the last of the memory
    Memory_Write32(0x80000038, 0x80000000 + DataPos);
    Memory_Write32(0x8000003C, RAM_24MB - DataPos);

    //allocate memory for our lookup structure
    ELFFiles = (ELFFileInfo*)malloc(sizeof(ELFFileInfo) * (FileCount + 1));
    memset(ELFFiles, 0, sizeof(ELFFileInfo) * (FileCount + 1));
    CurFilePos = 0;

    //reset our counts
    FileCount = 0;
    FileNameLen = 0;
/*#ifdef WIN32
    ELF_CreateFileStructureWindows(FilePath, FileInfo, &FileCount, FileNames, &FileNameLen, &TotalFileSize);
    (*((GCMFST *)(&Mem_RAM[DataPos]))).NameOffset = 0x01000000;
    (*((GCMFST *)(&Mem_RAM[DataPos]))).FileSize = FileCount + 1;
    (*((GCMFST *)(&Mem_RAM[DataPos]))).DiskAddr = 0;
#else*/
#pragma error("ELF_CreateFileStructure not modified for this OS!");
//#endif

    //bswap the memory around
    //	MemSwap = (u32 *)&Mem_RAM[DataPos + DataSize];
    //	FileNames = (char *)(&Mem_RAM[RAM_24MB]);
    for(; (u8*)FileNames < (u8*)&Mem_RAM[RAM_24MB]; FileNames+=4)
        *((u32 *)(FileNames)) = BSWAP32((*((u32 *)FileNames)));

    return 0x80000000 + DataPos;
}

/// Load an ELF (executable and linkable format)
int LoadELF(char* filename)
{
    u32 FSTStart;
    FILE *fElf = fopen(filename, "rb");

    if(!fElf)
        return E_ERR;

    fseek(fElf, 0, SEEK_END);
    int flen = ftell(fElf);
    fseek(fElf, 0, SEEK_SET);

    if((flen < sizeof(Elf32_Ehdr)) || (flen > 0x400000))    // >4MB not likely 
        return E_ERR;

    unsigned char * pElf = new unsigned char [ flen ] ;
    if(!pElf)
        return E_ERR;

    if( fread(pElf, 1, flen, fElf) != flen )
    {
        delete [] pElf;
        return E_ERR;
    }

    fclose(fElf);

    ////////////////////////////////////////////////////////////////////////////////////////////////

    LOG_NOTICE(TDVD, "ELF Load(): Opened: %s (%d bytes)", filename, flen );

    Elf32_Ehdr *pEhdr = (Elf32_Ehdr *)pElf;

    pEhdr->e_type		= BSWAP16(pEhdr->e_type);
    pEhdr->e_machine	= BSWAP16(pEhdr->e_machine);
    pEhdr->e_version	= BSWAP32(pEhdr->e_version);
    pEhdr->e_entry		= BSWAP32(pEhdr->e_entry);
    pEhdr->e_phoff		= BSWAP32(pEhdr->e_phoff);
    pEhdr->e_shoff		= BSWAP32(pEhdr->e_shoff);
    pEhdr->e_flags		= BSWAP32(pEhdr->e_flags);
    pEhdr->e_ehsize		= BSWAP16(pEhdr->e_ehsize);
    pEhdr->e_phentsize	= BSWAP16(pEhdr->e_phentsize);
    pEhdr->e_phnum		= BSWAP16(pEhdr->e_phnum);
    pEhdr->e_shentsize	= BSWAP16(pEhdr->e_shentsize);
    pEhdr->e_shnum		= BSWAP16(pEhdr->e_shnum);
    pEhdr->e_shstrndx	= BSWAP16(pEhdr->e_shstrndx);

    if((pEhdr->e_ident[0] != 0x7F) || (pEhdr->e_ident[1] != 0x45) ||	// 0x74 | 'E'
       (pEhdr->e_ident[2] != 0x4C) || (pEhdr->e_ident[3] != 0x46) ||	// 'L'  | 'F'
       (pEhdr->e_ident[EI_DATA] != ELFDATA2MSB)	||
       (pEhdr->e_ident[EI_CLASS] != ELFCLASS32)	||  
       (pEhdr->e_ident[EI_VERSION] != EV_CURRENT)	|| 
       (pEhdr->e_type != ET_EXEC)	||
       (pEhdr->e_machine != EM_PPC)	||
       (pEhdr->e_version != EV_CURRENT) ) {
            LOG_ERROR(TDVD, "This ELF File is NOT for IBM PPC Series Processors! (Gekko)");
            delete [] pElf;
            return E_ERR;
    }

    if((pEhdr->e_shentsize != sizeof(Elf32_Shdr)) ||
       (pEhdr->e_phentsize != sizeof(Elf32_Phdr)) ||
       (flen<(pEhdr->e_shoff+(pEhdr->e_shentsize*pEhdr->e_shnum))) ||
       (flen<(pEhdr->e_phoff+(pEhdr->e_phentsize*pEhdr->e_phnum)))) {
        delete [] pElf;
        return E_ERR;
    }

    u32 i=0;
    Elf32_Shdr *pShdr = (Elf32_Shdr *)pElf + pEhdr->e_shoff;
    Elf32_Phdr *pPhdr = (Elf32_Phdr *)pElf + pEhdr->e_phoff;

    LOG_NOTICE(TDVD, "ELF - entry:%X shnum:%X shoff:%X \n", pEhdr->e_entry, pEhdr->e_shnum, pEhdr->e_shoff);

    // ByteSwap Section Headers First
    //
    for( i=0; i<pEhdr->e_shnum; i++ )
    {
        pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

        pShdr->sh_name		= BSWAP32(pShdr->sh_name);
        pShdr->sh_type		= BSWAP32(pShdr->sh_type);
        pShdr->sh_flags		= BSWAP32(pShdr->sh_flags);
        pShdr->sh_addr		= BSWAP32(pShdr->sh_addr);
        pShdr->sh_offset	= BSWAP32(pShdr->sh_offset);
        pShdr->sh_size		= BSWAP32(pShdr->sh_size);
        pShdr->sh_link		= BSWAP32(pShdr->sh_link);
        pShdr->sh_info		= BSWAP32(pShdr->sh_info);
        pShdr->sh_addralign	= BSWAP32(pShdr->sh_addralign);
        pShdr->sh_entsize	= BSWAP32(pShdr->sh_entsize);
    }

    //	take care of symtab first
    //
    for( i=0; i<pEhdr->e_shnum; i++ )
    {
        pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

        if( SHT_SYMTAB == pShdr->sh_type )
        {
            symindex = pShdr->sh_link;
            symindex |= ((pShdr->sh_size / pShdr->sh_entsize) << 16);

            if(pShdr->sh_size < 0x80000)
                memcpy(symtab,(pElf+pShdr->sh_offset),pShdr->sh_size);
        }
    }

    //	take care of strtab second
    //
    for( i=0; i<pEhdr->e_shnum; i++ )
    {
        pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

        if( SHT_STRTAB == pShdr->sh_type )
        {
            if( (symindex&0xFFFF) == i ) {
                if(pShdr->sh_size < 0x80000) {
                    LOG_NOTICE(TDVD, "ELF: Found .strtab!\n");
                    memcpy(strtab,(pElf+pShdr->sh_offset),pShdr->sh_size);
                }
            } else
            {
                // can have more than one strtab, other than symtab/strtab too so check to see if its index into itself is ".shstrtab"
                char *szT = (char*)(pElf + pShdr->sh_offset + pShdr->sh_name);

                if(0==strcmp(szT,".shstrtab"))
                    if(pShdr->sh_size < 0x800) {
                        LOG_NOTICE(TDVD, "ELF: Found .shstrtab!\n");
                        memcpy(shstrtab,(pElf+pShdr->sh_offset),pShdr->sh_size);
                    }
            }
        }
    }

    //	take care of the rest
    //
    for( i=0; i<pEhdr->e_shnum; i++ )
    {
        pShdr = (Elf32_Shdr *)((u8*)pElf + pEhdr->e_shoff + pEhdr->e_shentsize * i);

        LOG_NOTICE(TDVD, "Segment[%i]: \"%s\" type:%X flags:%X offs:%X size:%X \n", i, 
            (char*)(shstrtab+pShdr->sh_name), pShdr->sh_type, pShdr->sh_flags, pShdr->sh_offset, pShdr->sh_size);

        switch( pShdr->sh_type )
        {
        case SHT_NULL:		break;
        case SHT_NOBITS:	break;
        case SHT_STRTAB:	break;
        case SHT_SYMTAB:	break;

        case SHT_PROGBITS:
            if( pShdr->sh_flags & SHF_ALLOC )	// WRITE=1 | ALLOC=2 | EXECINSTR=4
            {
                LOG_NOTICE(TDVD, "->\tLoaded To %X size:%X\n", pShdr->sh_addr, pShdr->sh_size);

                if( (0xC0000000 == (pShdr->sh_addr & 0xF0000000)) ||
                    (0x80000000 == (pShdr->sh_addr & 0xF0000000)) ||
                    (0x00000000 == (pShdr->sh_addr & 0xF0000000)) )
                {
                    for( u32 x = 0; x <= pShdr->sh_size; x++ ) {
                        Memory_Write8(pShdr->sh_addr + x, pElf[pShdr->sh_offset+x]);
                    }
                }
            }
            break;

        case SHT_NOTE:
        case SHT_RELA:
        case SHT_HASH:
        case SHT_DYNAMIC:
        case SHT_REL:
        case SHT_SHLIB:
        case SHT_DYNSYM:
        case SHT_INIT_ARRAY:
        case SHT_FINI_ARRAY:
        case SHT_PREINIT_ARRAY:
        case SHT_GROUP:
        case SHT_SYMTAB_SHNDX:

        default:
            LOG_NOTICE(TDVD, "ELF File Unknown SecHead[%x].type:%X \n", i, pShdr->sh_type);
            break;
        }
    }

    u32 EntryPoint = pEhdr->e_entry;
    delete [] pElf;

    //setup the current working directory
    //	getcwd(ELFCurrentDir, 256);
    //	sprintf(ELFCurrentDir, "%s\\dvdroot", ELFCurrentDir);

    //setup the files
    FSTStart = ELF_CreateFileStructure(filename);

    //setup the RealDVD entries
    RealDVDOpen = &ELFDVDOpen;
    RealDVDSeek = &ELFDVDSeek;
    RealDVDRead = &ELFDVDRead;
    RealDVDGetFileSize = &ELFDVDGetFileSize;
    RealDVDClose = &ELFDVDClose;
    RealDVDGetPos = &ELFDVDGetPos;
    RealDVDChangeDir = &ELFDVDChangeDir;

    Bootrom(FSTStart);

    Flipper_Open();
    cpu->Open(EntryPoint);

    HLE_ScanForPatches();
    return E_OK;
}

} // namespace
