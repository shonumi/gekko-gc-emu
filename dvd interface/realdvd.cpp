#include "RealDVD.h"

//actual RealDVD* vars at the end of the file to avoid having these external
DEFRealDVDRead(RealDVDReadDummy)
{
	printf("RealDVDRead not implemented\n");
	return 0;
}

DEFRealDVDSeek(RealDVDSeekDummy)
{
	printf("RealDVDSeek not implemented\n");
	return 0;
}

DEFRealDVDOpen(RealDVDOpenDummy)
{
	printf("RealDVDOpen not implemented\n");
	return 0;
}

DEFRealDVDGetFileSize(RealDVDGetFileSizeDummy)
{
	printf("RealDVDGetFileSize not implemented\n");
	return 0;
}

DEFRealDVDClose(RealDVDCloseDummy)
{
	printf("RealDVDClose not implemented\n");
	return 0;
}

DEFRealDVDGetPos(RealDVDGetPosDummy)
{
	printf("RealDVDGetPos not implemented\n");
	return 0;
}

DEFRealDVDChangeDir(RealDVDChangeDirDummy)
{
	printf("RealDVDChangeDir not implemented\n");
	return 0;
}

void ResetRealDVD()
{
	//reset all the functions
	RealDVDRead = &RealDVDReadDummy;
	RealDVDSeek = &RealDVDSeekDummy;
	RealDVDOpen = &RealDVDOpenDummy;
	RealDVDGetFileSize = &RealDVDGetFileSizeDummy;
	RealDVDClose = &RealDVDCloseDummy;
	RealDVDGetPos = &RealDVDGetPosDummy;
}

RealDVDReadFunc			RealDVDRead = &RealDVDReadDummy;
RealDVDSeekFunc			RealDVDSeek = &RealDVDSeekDummy;
RealDVDOpenFunc			RealDVDOpen = &RealDVDOpenDummy;
RealDVDGetFileSizeFunc	RealDVDGetFileSize = &RealDVDGetFileSizeDummy;
RealDVDCloseFunc		RealDVDClose = &RealDVDCloseDummy;
RealDVDGetPosFunc		RealDVDGetPos = &RealDVDGetPosDummy;
RealDVDChangeDirFunc	RealDVDChangeDir = &RealDVDChangeDirDummy;
