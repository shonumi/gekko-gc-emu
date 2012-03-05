// emu_browser.h
// (c) 2005,2006 Gekko Team

#ifndef _EMU_BROWSER_H_
#define _EMU_BROWSER_H_

////////////////////////////////////////////////////////////

#define ID_LISTVIEW						6993

#define PACKRGB555(r, g, b)				(u16)((((r)&0xf8)<<7)|(((g)&0xf8)<<2)|(((b)&0xf8)>>3))
#define PACKRGB565(r, g, b)				(u16)((((r)&0xf8)<<8)|(((g)&0xfc)<<3)|(((b)&0xf8)>>3))

//////////////////////////////////////////////////////////////////////

typedef struct _emuRomList
{
	char  name[MAX_PATH];                // ROM Name
	char  alttitle[MAX_PATH];             // Alternate ROM Name
	char  country;                       // Country Code
	char  filename[MAX_PATH];            // ROM File Name
	char  comment[MAX_PATH];             // Comments on ROM Functionality
	DWORD dwFileSize;                    // ROM File Size
} emuRomList;

//

//////////////////////////////////////////////////////////////////////

void BrowserCreateListView(void);
bool BrowserUpdateROMList(void);
void BrowserReloadWindow(void);
void BrowserKill(void);
void BrowserSelectDir(void) ;
u8 GetBnrChecksum(void *banner);

//////////////////////////////////////////////////////////////////////

extern		emuRomList	romList;
static		int			statwidths[] = {100, -1};

//

#endif