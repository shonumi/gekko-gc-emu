#ifndef ELF_HEADER
#define ELF_HEADER
#include "..\emu.h"

DEFRealDVDRead(ELFDVDRead);
DEFRealDVDSeek(ELFDVDSeek);
DEFRealDVDOpen(ELFDVDOpen);
DEFRealDVDGetFileSize(ELFDVDGetFileSize);
DEFRealDVDClose(ELFDVDClose);
DEFRealDVDGetPos(ELFDVDGetPos);
DEFRealDVDChangeDir(ELFDVDChangeDir);
u32 ELF_CreateFileStructure(char *ELFFileName);

#endif