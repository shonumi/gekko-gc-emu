#ifndef GCM_HEADER
#define GCM_HEADER
#include "..\emu.h"

#define		GCMFILEID	0x314D4347

//info about the files in the GCM
typedef struct _GCMFST
{
	u32		NameOffset;
	u32		DiskAddr;
	u32		FileSize;
} GCMFST;

typedef struct _GCMFSTHeader
{
	u32		Offset;
	u32		Size;
	u32		MaxSize;
	u32		MemLocation;
} GCMFSTHeader;

//our data about the FST
typedef struct _GCMFileData
{
	char *			Filename;
	u32				DiskAddr;
	u32				FileSize;			//size of a file
	u32				FileCount;			//file count in a directory
	u32				IsDirectory;
	_GCMFileData *	FileList;
	_GCMFileData *	Parent;
} GCMFileData;

//info about the pointer passed in
typedef struct	_GCMFileInfo
{
	u32					ID;				//should be "GCM1", GCMFILEID
	GCMFileData *		FileData;		//file data
	u32					CurPos;			//current pointer in the GCM
	_GCMFileInfo *		PrevPtr;		//prev and next used only during cleanup
	_GCMFileInfo *		NextPtr;		//of memory
} GCMFileInfo;

extern char	CurrentGameName[992];
extern char	CurrentGameCRC[7];

//all filenames in the FST
extern char *FileNames;

//poitner to the FST
extern GCMFileData *	FST;

//pointers to open files
extern GCMFileInfo *	FilePtrs;
extern GCMFileInfo *	LowLevelPtr;
extern GCMFileData *	GCMCurDir;

u32 AdjustFSTCounts(GCMFST *CurGCMFSTData, u32 *CurIndex, u32 LastIndex);
void ParseFSTTree(GCMFileData *FSTEntry, GCMFileData *RootFSTEntry, GCMFST **CurEntry, char *Filenames, u32 *Count, GCMFileData *ParentFST);
GCMFileData *FindFSTEntry(GCMFileData *CurEntry, char *Filename);
GCMFileData *ChangeDirEntry(GCMFileData *CurEntry, char *Filename);

#endif