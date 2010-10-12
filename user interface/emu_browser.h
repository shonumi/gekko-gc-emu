// emu_browser.h
// (c) 2005,2006 Gekko Team

#ifndef _EMU_BROWSER_H_
#define _EMU_BROWSER_H_

////////////////////////////////////////////////////////////

#define SIZE_OF_GCM_HEADER				0x440
#define SIZE_OF_GCM_BANNER				0x1960

#define DVD_BANNER_FILENAME				"opening.bnr"

#define DVD_BANNER_WIDTH				96
#define DVD_BANNER_HEIGHT				32

#define DVD_BANNER_ID					'BNR1'  // JP/US
#define DVD_BANNER_ID2					'BNR2'  // EU

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

// JAP/USA
typedef struct DVDBanner
{
    u32     id;		// 'BNR1'											
    u32     padding[7];
    u8      image[2 * DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT];	
    u8      shortTitle[32];										
    u8      shortMaker[32];										
    u8      longTitle[64];										
    u8      longMaker[64];										
    u8      comment[128];										
} DVDBanner;

// EUR
typedef struct DVDBanner2
{
    u32     id;		// 'BNR2'
    u32     padding[7];
    u8      image[2 * DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT];

    struct
    {
        u8  shortTitle[32];
        u8  shortMaker[32];
        u8  longTitle[64];
        u8  longMaker[64];
        u8  comment[128];
    } comments[6];
} DVDBanner2;

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