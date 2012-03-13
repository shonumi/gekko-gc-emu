// emu_browser.cpp
// (c) 2005,2006 Gekko Team

#include "emu.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
//

HIMAGELIST		hBanner;				// Banner icons
emuRomList		romList;				// Stores rom browser list info.
char			szBuffer[1024];			// Temporary string buffer.

//////////////////////////////////////////////////////////////////////

// adds a banner to the browser
static BOOL BrowserAddBanner(u8 *banner, int *bA, int *bB)
{
    int			pack = 1;
    int			width = DVD_BANNER_WIDTH;
    int			height = DVD_BANNER_HEIGHT;
    HDC			hdc = CreateDC((LPWSTR)"DISPLAY", NULL, NULL, NULL);
    int			bitdepth = GetDeviceCaps(hdc, BITSPIXEL);
    int			bpp = bitdepth / 8;
    int			bcount = width * height * bpp;
    int			tiles  = (DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT) / 16;    
    u8			*imageA, *imageB, *ptrA, *ptrB;
    f64			rgb[3], rgbh[3];
    u16			*tile  = (u16 *)banner, *ptrA16, *ptrB16;
    u32			r, g, b, a, rh, gh, bh;
    int			row = 0, col = 0, pos;

    imageA = (u8 *)malloc(bcount);
    if(imageA == NULL)
    {
        DeleteDC(hdc);
        return FALSE;
    }
    imageB = (u8 *)malloc(bcount);
    if(imageB == NULL)
    {
        free(imageA);
        DeleteDC(hdc);
        return FALSE;
    }

    DWORD backcol = GetSysColor(COLOR_WINDOW);
    rgb[0] = (f64)GetRValue(backcol);
    rgb[1] = (f64)GetGValue(backcol);
    rgb[2] = (f64)GetBValue(backcol);

    backcol = GetSysColor(COLOR_HIGHLIGHT);
    rgbh[0] = (f64)GetRValue(backcol);
    rgbh[1] = (f64)GetGValue(backcol);
    rgbh[2] = (f64)GetBValue(backcol);

    // Convert RGB5A3 -> RGBA
    for(int i=0; i<tiles; i++, tile+=16)
    {
        for(int j=0; j<4; j++)
        for(int k=0; k<4; k++)
        {
            u16 p, ph;

            p = tile[j * 4 + k];
            ph = p = (p << 8) | (p >> 8);
            if(p >> 15)
            {
                r = (p & 0x7c00) >> 10;
                g = (p & 0x03e0) >> 5;
                b = (p & 0x001f);
                r = (u8)((r << 3) | (r >> 2));
                g = (u8)((g << 3) | (g >> 2));
                b = (u8)((b << 3) | (b >> 2));
                rh = r, gh = g, bh = b;
            }
            else
            {
                r = (p & 0x0f00) >> 8;
                g = (p & 0x00f0) >> 4;
                b = (p & 0x000f);
                a = (p & 0x7000) >> 12;

                rh = (u8)((f64)(16*r) + rgbh[0]);
                gh = (u8)((f64)(16*g) + rgbh[1]);
                bh = (u8)((f64)(16*b) + rgbh[2]);

                r = g = b = 0;
            }

            pos = bpp * ((row + j) * width + (col + k));
			

            ptrA16 = (u16 *)&imageA[pos];
            ptrB16 = (u16 *)&imageB[pos];
            ptrA   = &imageA[pos];
            ptrB   = &imageB[pos];

            if(bitdepth == 8)
            {
                *ptrA++ =
                *ptrB++ = (u8)(r | g ^ b);
            }
            else if(bitdepth == 16)
            {
                if(pack)        // 565
                {
                    p = PACKRGB565(r, g, b);
                    ph= PACKRGB565(rh, gh, bh);
                }
                else            // 555 (on old boxes)
                {
                    p = PACKRGB555(r, g, b);
                    ph= PACKRGB555(rh, gh, bh);
                }

                *ptrA16 = p;
                *ptrB16 = ph;
            }
            else
            {
                *ptrA++ = (u8)b;
                *ptrA++ = (u8)g;
                *ptrA++ = (u8)r;

                *ptrB++ = (u8)bh;
                *ptrB++ = (u8)gh;
                *ptrB++ = (u8)rh;
            }
        }

        col += 4;
        if(col == DVD_BANNER_WIDTH)
        {
            col = 0;
            row += 4;
        }
    }

    HBITMAP hbm = CreateCompatibleBitmap(hdc, width, height);
    if(hbm == NULL)
    {
        DeleteDC(hdc);
        free(imageA), free(imageB);
        return FALSE;
    }

	// Normal icon
    SetBitmapBits(hbm, bcount, imageA);     
    *bA = ImageList_Add(hBanner, hbm, NULL);

	// Highlighted icon
    SetBitmapBits(hbm, bcount, imageB);     
    *bB = ImageList_Add(hBanner, hbm, NULL);

    // Clean up
    DeleteObject(hbm);
    DeleteDC(hdc);
    free(imageA), free(imageB);

    return true;
}

// creates the browser list
void BrowserCreateListView(void)
{
	LV_COLUMN			lvC;			// Listview column structure
	RECT				rcl;			// Window size

	// Get size and position of parent window
	GetClientRect(wnd.hWnd, &rcl);
#pragma todo("reimplement listview")
	// Create the listview handle
	/*wnd.hWndList = CreateWindowExW(
    (DWORD)WC_LISTVIEW,
    (LPWSTR)"",
    (LPWSTR)"",
    WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT | WS_EX_CLIENTEDGE,
    0,
    0,
    rcl.right - rcl.left,
    rcl.bottom - rcl.top,
    wnd.hWnd,
    (HMENU)ID_LISTVIEW,
    wnd.hInstance,
    NULL);*/


	// Initialize listview sections

	lvC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;

	lvC.fmt = LVCFMT_LEFT;
	lvC.cx = rcl.right;
	lvC.pszText = (LPWSTR)"Debug Output";
	ListView_InsertColumn( wnd.hWndDbg, 0, &lvC );

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvC.fmt = LVCFMT_LEFT;
	lvC.iSubItem = 0;
	lvC.cx = 96;
	lvC.pszText = (LPWSTR)"Icon";
	ListView_InsertColumn( wnd.hWndList, 0, &lvC);

	lvC.fmt = LVCFMT_LEFT;
	lvC.iSubItem = 1;
	lvC.cx = 280;
	lvC.pszText = (LPWSTR)"Name";
	ListView_InsertColumn( wnd.hWndList, 1, &lvC);

	lvC.fmt = LVCFMT_CENTER;
	lvC.iSubItem = 2;
	lvC.cx = 60;
	lvC.pszText = (LPWSTR)"Country";
	ListView_InsertColumn( wnd.hWndList, 2, &lvC);

	lvC.fmt = LVCFMT_CENTER;
	lvC.iSubItem = 3;
	lvC.cx = 70;
	lvC.pszText = (LPWSTR)"Size";
	ListView_InsertColumn( wnd.hWndList, 3, &lvC);

	lvC.fmt = LVCFMT_LEFT;
	lvC.iSubItem = 4;
	lvC.cx = 300;
	lvC.pszText = (LPWSTR)"Notes";
	ListView_InsertColumn( wnd.hWndList, 4, &lvC);		// Small typo

	lvC.fmt = LVCFMT_LEFT;
	lvC.iSubItem = 5;
	lvC.cx = 0;
	lvC.pszText = (LPWSTR)"Filename";
	ListView_InsertColumn( wnd.hWndList, 5, &lvC);
}

// scans a path for browser files
void BrowserScanPath(char * _path_to_scan)
{
	WIN32_FIND_DATA		FileData;					// File Find Data Structure
	HANDLE				hSearch = NULL;				// File Search Handle
	HANDLE				hFile = NULL;				// Handle to a File
	LV_ITEM				lvI;						// List View Item Structure
	LPCTSTR				defString = (LPCWSTR)"\0";			// Default String
	LPCTSTR				szFilter = (LPCWSTR)"*.gcm\0*.iso\0";
	DWORD				nBytesRead = 0;
	EmuRomInfo			romIni;						// Data from game list
	int					i = 0, a = 0, b = 0;
	int					nBanner = 0, nCount = 0, nFilter = 1;
	u8					Banner[SIZE_OF_GCM_BANNER];
	char				Header[SIZE_OF_GCM_HEADER], gameCode[7];

	// Get available gcm information
	for( i = 0; i <= nFilter; i++ )
	{
		// Find first filename matching the search criteria
		char filter[MAX_PATH];
		strcpy(filter,g_szRomPath);
		strcat(filter,(const char *)szFilter + (i * 6) );

		hSearch = FindFirstFile( (LPCWSTR)filter , &FileData );

		if( hSearch != INVALID_HANDLE_VALUE )
		{
			while( GetLastError() != ERROR_NO_MORE_FILES )
			{
				// Create filename path
				strcpy(romList.filename,_path_to_scan);
				strcat(romList.filename,(const char *)FileData.cFileName);

				// Initialize new listview item
				lvI.mask = LVIF_IMAGE;
				lvI.iItem = nCount;
				lvI.iSubItem = 0;

				// Clear GCM entry arrays
				ZeroMemory(Banner, SIZE_OF_GCM_BANNER);
				ZeroMemory(Header, SIZE_OF_GCM_HEADER);

				// Load GCM information
				if(dvd::ReadGCMInfo(romList.filename, &romList.dwFileSize, Banner, Header))
				{
					// Load and Decode Banner
					ListView_SetImageList( wnd.hWndList, hBanner, LVSIL_SMALL );

					u8	_banner[SIZE_OF_GCM_BANNER];
					memcpy(&_banner[0], &Banner[0x20], SIZE_OF_GCM_BANNER - 0x20);
					BrowserAddBanner(_banner, &a, &b);	
					
					lvI.iImage = nBanner;
					nBanner+=2;
					
					ListView_InsertItem(wnd.hWndList, &lvI);

					romList.country = Header[0x0003];
				
					ZeroMemory(romList.name, sizeof( romList.name ));
					CopyMemory(romList.name, Header + 32, 100);

					// Load the gamecode
					HLE_GetGameCRC(gameCode, (u8 *)&Header[0], GetBnrChecksum(Banner));

					switch(romList.country)
					{
					case 0x45:
						ListView_SetItemText(wnd.hWndList, nCount, 2, (LPWSTR)"USA");
						break;

					case 0x4A:
						ListView_SetItemText(wnd.hWndList, nCount, 2, (LPWSTR)"JAP");
						break;

					default:
						ListView_SetItemText(wnd.hWndList, nCount, 2, (LPWSTR)"PAL");	  // Will be more precise later (with ini file ?)
						break;
					}

					romList.alttitle[0] = '\0';
					romList.comment[0] = '\0';

					ZeroMemory(&romIni, sizeof(romIni));

					findINIEntry(gameCode, &romIni, 1);
                    int sizecomment = min (strlen (romIni.comments), MAX_PATH-1);  // romList.comment is MAXPATH max
					strncpy(romList.comment, romIni.comments,sizecomment);
                    romList.comment [sizecomment] = '\0';
					ListView_SetItemText(wnd.hWndList, nCount, 4, (LPWSTR)romList.comment);

					strcpy(romList.alttitle, romIni.title_alt);  
					if(strlen(romList.alttitle))
					{
						ListView_SetItemText(wnd.hWndList, nCount, 1, (LPWSTR)romList.alttitle);
					}else{
						ListView_SetItemText(wnd.hWndList, nCount, 1, (LPWSTR)romList.name);
					}

					wsprintf((LPWSTR)szBuffer, (LPCWSTR)"%iMbit", romList.dwFileSize / 131072);
					ListView_SetItemText(wnd.hWndList, nCount, 3, (LPWSTR)szBuffer);

					ListView_SetItemText(wnd.hWndList, nCount, 5, (LPWSTR)romList.filename);

					// Increment rom count
					nCount++;
				}else{
					MessageBox( wnd.hWnd, 
						(LPCWSTR)"Unable to read ISO for rom browser!",
						(LPCWSTR)"Error",
						MB_ICONEXCLAMATION | MB_OK 
					);
				}

				// Get next available rom file.
				FindNextFile(hSearch, &FileData);
			}
		}

		// End the search
		FindClose(hSearch);
	}
}

// updates the rom browser list
bool BrowserUpdateROMList(void)
{
	// Reinitialize the banner image list
    ImageList_Remove(hBanner, -1);
    ImageList_Destroy(hBanner);
	hBanner = ImageList_Create(DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT, ILC_COLOR24, 10, 10);

	// Set window title
	wsprintf((LPWSTR)szBuffer, (LPCWSTR)"%s - Refreshing Available ROM List", APP_NAME);
	SetWindowText(wnd.hWnd, (LPCWSTR)szBuffer);

	// Scan for GCM
	BrowserScanPath(g_szRomPath);

	// Reset Window Title
	wsprintf( (LPWSTR)szBuffer, (LPCWSTR)"%s", APP_NAME);
	SetWindowText( wnd.hWnd, (LPCWSTR)szBuffer );

	return true;
}

// destroys the rom browser
void BrowserKill(void)
{
    ImageList_Remove(hBanner, -1);
    ImageList_Destroy(hBanner);
    hBanner = NULL;

    ListView_DeleteAllItems(wnd.hWndList);
    DestroyWindow(wnd.hWndList);
    wnd.hWndList = NULL;
    SetFocus(wnd.hWnd);
}

// rescans for roms
void BrowserReloadWindow(void)
{
	// Make sure window has been created
	if(!wnd.hWnd) return;

	// Kill old browser
	BrowserKill();

	// Create new listview
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	BrowserCreateListView();
	BrowserUpdateROMList();
	SetCursor(LoadCursor(NULL,IDC_ARROW));

	// Create new status bar
	/*wnd.hWndStatus = CreateWindowEx( 0L,
		STATUSCLASSNAME,
		"",
		WS_CHILD | WS_BORDER |
		WS_VISIBLE | SBS_SIZEGRIP,
		0, 0, 0, 0,
		wnd.hWnd,
		(HMENU)ID_STATUSBAR,
		wnd.hInstance,
		NULL 
	);

	if(wnd.hWndStatus == NULL)
		MessageBox( wnd.hWnd, 
			"Status Bar not Created",
			"Error",
			MB_ICONEXCLAMATION | MB_OK 
		);*/
}

// selects the browser scan directory
void BrowserSelectDir(void) 
{
	char Buffer[MAX_PATH], Directory[255];
	LPITEMIDLIST pidl;
	BROWSEINFO bi;

	bi.hwndOwner = wnd.hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = (LPWSTR)Buffer;
	bi.lpszTitle = (LPCWSTR)"Select current Rom Directory";
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
		if (SHGetPathFromIDList(pidl, (LPWSTR)Directory)) {
			int len = strlen(Directory);

			if (Directory[len - 1] != '\\') {
				strcat(Directory,"\\");
			}

			strcpy(g_szRomPath, Directory);
			BrowserReloadWindow();

			sprintf(Buffer, "%sGekko.ini", ProgramDirectory);
			WritePrivateProfileString((LPCWSTR)"System",(LPCWSTR)"ROM Path", (LPCWSTR)g_szRomPath, (LPCWSTR)Buffer);
		}
	}
}