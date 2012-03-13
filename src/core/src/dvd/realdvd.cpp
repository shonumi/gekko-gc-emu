#include "realdvd.h"

/// Frontend interface for DVD/ROM loading
namespace dvd {

//actual RealDVD* vars at the end of the file to avoid having these external
DEFRealDVDRead(RealDVDReadDummy)
{
	LOG_ERROR(TDVD, "RealDVDRead not implemented\n");
	return 0;
}

DEFRealDVDSeek(RealDVDSeekDummy)
{
	LOG_ERROR(TDVD, "RealDVDSeek not implemented\n");
	return 0;
}

DEFRealDVDOpen(RealDVDOpenDummy)
{
	LOG_ERROR(TDVD, "RealDVDOpen not implemented\n");
	return 0;
}

DEFRealDVDGetFileSize(RealDVDGetFileSizeDummy)
{
	LOG_ERROR(TDVD, "RealDVDGetFileSize not implemented\n");
	return 0;
}

DEFRealDVDClose(RealDVDCloseDummy)
{
	LOG_ERROR(TDVD, "RealDVDClose not implemented\n");
	return 0;
}

DEFRealDVDGetPos(RealDVDGetPosDummy)
{
	LOG_ERROR(TDVD, "RealDVDGetPos not implemented\n");
	return 0;
}

DEFRealDVDChangeDir(RealDVDChangeDirDummy)
{
	LOG_ERROR(TDVD, "RealDVDChangeDir not implemented\n");
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

} // namespace

