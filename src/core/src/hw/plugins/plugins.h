// plugins.h
// (c) 2005,2006 Gekko Team

#ifndef _PLUGINS_H_
#define _PLUGINS_H_

////////////////////////////////////////////////////////////

#include "dolwinspec.h"

extern PluginData           plug;

extern REGISTERPLUGIN       RegisterPlugin;

extern GXOPEN               GXOpen;
extern GXCLOSE              GXClose;
extern GXWRITEFIFO          GXWriteFifo;
extern GXSETTOKENS          GXSetTokens;
extern GXCONFIGURE          GXConfigure;
extern GXABOUT              GXAbout;
extern GXSAVELOAD           GXSaveLoad;

extern PADOPEN              PADOpen;
extern PADCLOSE             PADClose;
extern PADREADBUTTONS       PADReadButtons;
extern PADSETRUMBLE         PADSetRumble;
extern PADCONFIGURE         PADConfigure;
extern PADABOUT             PADAbout;
extern PADSAVELOAD          PADSaveLoad;

void   PS_Open();
void   PS_Close();

////////////////////////////////////////////////////////////

#endif
