#include "../emu.h"

//header for any RealDVD* calls
//these functions are overwritten depending on what loader wants them
//MemPtr assumes physical memory location, not a virtual position
#define DEFRealDVDRead(Func)		u32 Func(u32 FilePtr, u32 *MemPtr, u32 Len)
#define DEFRealDVDSeek(Func)		u32 Func(u32 FilePtr, u32 SeekPos, u32 SeekType)
#define DEFRealDVDOpen(Func)		u32 Func(char *filename)
#define DEFRealDVDGetFileSize(Func)	u32 Func(u32 FilePtr)
#define DEFRealDVDClose(Func)		u32 Func(u32 FilePtr)
#define DEFRealDVDGetPos(Func)		u32 Func(u32 FilePtr)
#define DEFRealDVDChangeDir(Func)	u32	Func(char *ChangeDirPath)

typedef u32(*RealDVDReadFunc)(u32 FilePtr, u32 *MemPtr, u32 Len);
typedef u32(*RealDVDSeekFunc)(u32 FilePtr, u32 SeekPos, u32 SeekType);
typedef u32(*RealDVDOpenFunc)(char *filename);
typedef u32(*RealDVDGetFileSizeFunc)(u32 FilePtr);
typedef u32(*RealDVDCloseFunc)(u32 FilePtr);
typedef u32(*RealDVDGetPosFunc)(u32 FilePtr);
typedef u32(*RealDVDChangeDirFunc)(char *ChangeDirPath);

extern RealDVDReadFunc			RealDVDRead;
extern RealDVDSeekFunc			RealDVDSeek;
extern RealDVDOpenFunc			RealDVDOpen;
extern RealDVDGetFileSizeFunc	RealDVDGetFileSize;
extern RealDVDCloseFunc			RealDVDClose;
extern RealDVDGetPosFunc		RealDVDGetPos;
extern RealDVDChangeDirFunc		RealDVDChangeDir;

#define REALDVDSEEK_START	0
#define REALDVDSEEK_CUR		1
#define REALDVDSEEK_END		2

//a unique ID incase we wish to read the DVD directly instead of a file from the dvd
//sending this ID to seek or read adjusts the read of the low level data
//sending this to the close function will terminate and cleanup the area controlling
//the RealDVD calls
#define REALDVD_LOWLEVEL	0xFFFFFFFF

//used when a rom is being opened
void ResetRealDVD();