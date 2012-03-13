// emu_ini.h
// (c) 2005,2006 Gekko Team

#ifndef _EMU_INI_H_
#define _EMU_INI_H_

////////////////////////////////////////////////////////////

typedef struct t_EmuRomInfo
{
	char title_alt[1024];
	char comments[1024];
}EmuRomInfo;

extern EmuRomInfo RomInfo;
extern bool DisableINIPatches;

////////////////////////////////////////////////////////////

int findINIEntry(char *gamecode, EmuRomInfo *rominfo, bool SkipPatch);

//

#endif