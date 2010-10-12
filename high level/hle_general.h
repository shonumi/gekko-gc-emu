// hle_general.h
// (c) 2005,2006 Gekko Team

#ifndef _HLE_GENERAL_H_
#define _HLE_GENERAL_H_

////////////////////////////////////////////////////////////
// DVD
////////////////////////////////////////////////////////////

extern u32 dvdfilehandle[128];
extern u32 filehandle_ptr[128];

struct DVDDiskID
{
    char      gameName[4];
    char      company[2];
    u8        diskNumber;
    u8        gameVersion;
    u8        streaming;
    u8        streamingBufSize; // 0 = default
    u8        padding[22];      // 0's are stored
};

struct DVDCommandBlock;
typedef void (*DVDCBCallback)(s32 result, DVDCommandBlock* block);

struct DVDCommandBlock
{
    DVDCommandBlock* next;
    DVDCommandBlock* prev;
    u32          command;
    s32          state;
    u32          offset;
    u32          length;
    void*         addr;
    u32          currTransferSize;
    u32          transferredSize;
    DVDDiskID*    id;
    DVDCBCallback callback;
    void*         userData;
};

struct DVDFileInfo
{
	DVDCommandBlock cb;
    u32 startAddr;      // disk address of file
    u32 length;         // file size in bytes
    u32 callback;
};

////////////////////////////////////////////////////////////

static u32 getDVDFileHandle(u32 ptr_addr)
{
	for(int i=0;i<128;i++)
	{
		if(ptr_addr == filehandle_ptr[i])
			return dvdfilehandle[i];
	}

	return 0;
}

static void setDVDFileHandle(u32 ptr_addr, u32 file)
{
	for(int i=0;i<128;i++)
	{
		if(filehandle_ptr[i] == 0)
		{
			filehandle_ptr[i] = ptr_addr;
			dvdfilehandle[i] = file;
			return;
		}
	}
}

static void delDVDFileHandle(u32 ptr_addr)
{
	for(int i=0;i<128;i++)
	{
		if(filehandle_ptr[i] == ptr_addr)
		{
			filehandle_ptr[i] = 0;
			dvdfilehandle[i] = NULL;
			return;
		}
	}
}

////////////////////////////////////////////////////////////

#endif