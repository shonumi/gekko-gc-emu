// emu_win.cpp
// (c) 2005,2006 Gekko Team

#include "../emu.h"

//

emuWnd wnd;
emuWnd wndAbout;

char g_szRomPath[1024];

//////////////////////////////////////////////////////////////////////

BOOL CALLBACK WIN_About_Proc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParan )
{
	switch( uMsg ) {
	case WM_INITDIALOG:	return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
            case IDC_ABOUT_OK:
                wndAbout.active = FALSE;
		        DestroyWindow(hWnd);
                return TRUE;
        }
        return TRUE;
	case WM_CLOSE:
	case WM_DESTROY:
		wndAbout.active = FALSE;
		DestroyWindow(hWnd);
		return TRUE; 
	default: break;
	}
	return FALSE;
}

void WIN_About_Open(void)
{
	if(wndAbout.active == false)
	{
		wndAbout.active = true;

		wndAbout.hWnd = CreateDialog( wnd.hInstance, MAKEINTRESOURCE(IDD_ABOUT_GEKKO), NULL, WIN_About_Proc);
	}
}

//////////////////////////////////////////////////////////////////////

void WIN_Framerate(void)
{
	static u32 swaps = 0, last = 0;
	static float fps = 0;
	u32 t = GetTickCount ();
	static u64 opcount = 0;
	s64 ops = 0;
	f32 mips = 0;
	static double opsspeed = 0;

	swaps++;
		
	if(t - last > 1000)
	{
		fps = (float) swaps / (0.001 * (t - last));
		ops = (cpu->GetTicks() - opcount);
		opsspeed = (((s64)ops) / (float)(cpu->GetTicksPerSecond()));
		mips = ((float)ops)/1000000;
		swaps = 0;
		last = t;
		opcount = cpu->GetTicks();

		char str1[100];
		char str2[15];
		switch(cpu->IsCPUCompareActive())
		{
			case 1:	//client
				sprintf(str2, " - Fucked Up ");
				break;

			case 2: //server
				sprintf(str2, " - Gnarly ");
				break;

			default:
				sprintf(str2, "");
				break;
		}

		sprintf(str1, APP_NAME "%s - %03.02f Fps - %03.02f MIPS (%03.02f%%)", str2, fps, mips, opsspeed*10);
		SetWindowText(wnd.hWnd, str1);
	}
}

void WIN_LoadFromString(char _filename[])
{
	char *ext;
	char ext2[64];
	char str[256], *c;
	char *PipeData;
	u32 FileLoaded = 1;

	c = strrchr (_filename, '/');
	if (c)
		strcpy (str, c + 1);
	else
	{
		c = strrchr (_filename, '\\');
		if (c)
			strcpy (str, c + 1);
		else
			strcpy (str, _filename);
	}

	c = strrchr (str, '.');
	if (c)
    {
	  strcpy (ext2, c + 1);
      ext=strlwr (ext2);   // For the checks
    }
	
	if(cpu->IsCPUCompareActive() == 2)
	{
		PipeData = (char *)malloc(strlen(_filename) + 2);
		PipeData[0] = 'F';
		memcpy(&PipeData[1], _filename, strlen(_filename) + 1);
		cpu->SendPipeData(PipeData, strlen(_filename) + 2);
		Sleep(1000);
		free(PipeData);
	}

	if (0 == stricmp (ext, "dol"))
		DOL_Open(_filename);
	else if (0 == stricmp (ext, "elf"))
		ELF_Open(_filename);
	else if (0 == stricmp (ext, "gcm") || 0 == stricmp (ext, "iso"))
		GCM_Open(_filename);
	else if (0 == stricmp (ext, "dmp"))
	{
		strcpy(ext, c - 3);
		if(0 == stricmp(ext, "gcm.dmp"))
			GCMDMP_Open(_filename);
	}
	else
	{
		printf(".Emu: ERROR: Unsupported ROM type!\n");
		FileLoaded = 0;
	}

	if(FileLoaded)
	{
		sprintf(g_szRomPath,"%s",_filename);
		
		for(int levels=0;levels<2;levels++)
		{
			for(int i=strlen(g_szRomPath);i>0;i--)
			{
				if(g_szRomPath[i]=='\\') 
				{
					g_szRomPath[i]='\0';
					break;
				}
			}
		}
		if(strstr(g_szRomPath,"SDK") != NULL)
		{
			u32 offset = strstr(g_szRomPath,"SDK") - g_szRomPath;
			for(int i=offset;i<strlen(g_szRomPath);i++)
			{
				if(g_szRomPath[i]=='\\')
				{
					g_szRomPath[i]='\0';
					strcat(g_szRomPath,"\\dvddata\\");
					break;
				}
			}
		}
	}
}

void WIN_OpenFile(void)
{
	char Filter[1024] = { "GameCube Executable GCGM/DOL/ELF/BIN/ISO/DMP\0*.gcm;*.dol;*.elf;*.bin;*.sio;*.gcm.dmp\0" "All Files (*.*)\0*.*\0" "" };
	char FileTitle[1024],File[1024]="",path[1024];

	OPENFILENAME ROM;		
	memset(&ROM, 0x00, sizeof(ROM));
	ROM.lStructSize = sizeof(ROM);
	ROM.hwndOwner = wnd.hWnd;
	ROM.lpstrFilter = Filter;
	ROM.lpstrCustomFilter = (LPTSTR)NULL;
	ROM.nMaxCustFilter = 0L;
	ROM.nFilterIndex = 1L;
	ROM.lpstrFile = File;
	ROM.nMaxFile = sizeof(File);
	ROM.lpstrFileTitle = FileTitle;
	ROM.nMaxFileTitle = sizeof(FileTitle);

	if(path) ROM.lpstrInitialDir = path;
	else ROM.lpstrInitialDir = ".";
	
	ROM.lpstrTitle = "Open GameCube Executable";
	ROM.Flags = OFN_ENABLESIZING|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
	ROM.nFileOffset = 0;
	ROM.nFileExtension = 0;
	ROM.lpstrDefExt = "*.gcm";
	ROM.lCustData = 0;
	
	if((GetOpenFileName(&ROM)) == 0)
	{
		printf("WIN_OpenFile() Failed!\n");
		return;
	}

//	WIN_LoadFromString(ROM.lpstrFile);
	strcpy(LoadedGameFile, ROM.lpstrFile);
	emu.status = EMU_LOAD;

/*
	sprintf(g_szRomPath,"%s",ROM.lpstrFile);
	
	for(int levels=0;levels<2;levels++)
	{
		for(int i=strlen(g_szRomPath);i>0;i--)
		{
			if(g_szRomPath[i]=='\\') 
			{
				g_szRomPath[i]='\0';
				break;
			}
		}
	}
	if(strstr(g_szRomPath,"SDK") != NULL)
	{
		u32 offset = strstr(g_szRomPath,"SDK") - g_szRomPath;
		for(int i=offset;i<strlen(g_szRomPath);i++)
		{
			if(g_szRomPath[i]=='\\')
			{
				g_szRomPath[i]='\0';
				strcat(g_szRomPath,"\\dvddata\\");
				break;
			}
		}
	}
*/
}

LRESULT CALLBACK WIN_Proc(	HWND	hWnd,
							UINT	uMsg,
							WPARAM	wParam,
							LPARAM	lParam)
{
	NMLISTVIEW		*pNm;
	char			szbuff[512];
	char			str1[1024];
	RECT			rect;
	int				x, y, ReturnCode;
	SHELLEXECUTEINFO sei;
	char			PipeData;

	switch (uMsg)
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_KEYDOWN:
		{
			//if the compare is active then store the key press only if we are the master
			x = cpu->IsCPUCompareActive();
			if(x == 1)
			{
				//client, ignore the key
			}
			else if(x == 2)
			{
				//master, accept the key but buffer it first
				emu.compkeys[wParam] = TRUE;
				emu.keychange = TRUE;
			}
			else
				emu.keys[wParam] = TRUE;
			return 0;
		}

		case WM_KEYUP:
		{
			//if the compare is active then store the key press only if we are the master
			x = cpu->IsCPUCompareActive();
			if(x == 1)
			{
				//client, ignore the key
			}
			else if(x == 2)
			{
				//master, accept the key but buffer it first
				emu.compkeys[wParam] = FALSE;
				emu.keychange = TRUE;
			}
			else
				emu.keys[wParam] = FALSE;
			return 0;
		}

		case WM_SIZE:
		{
			if(emu.status == EMU_HALTED) BrowserReloadWindow();

			return 0;
		}

		case WM_NOTIFY:
		{
			if( wParam != ID_LISTVIEW )
				break;

			pNm = (NMLISTVIEW *)lParam;

            int ligne = pNm->iItem ;
            switch( pNm->hdr.code )
            {
 				case NM_CLICK:
					if ( ligne < 0)	 // We hit a sub item
					{
						// Find the associated Item
			            LVHITTESTINFO hi;
			            hi.pt.x = pNm->ptAction.x;
        			    hi.pt.y = pNm->ptAction.y;
		        	    hi.flags = LVHT_ONITEM ;
        		    	SNDMSG(wnd.hWndList, LVM_SUBITEMHITTEST, 0, (LPARAM)&hi);
			            ligne = hi.iItem;
					}
					if ( ligne >= 0)
						ListView_SetItemState(wnd.hWndList, ligne, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
					break;

				case NM_DBLCLK:
          if ( ligne < 0)	 // We hit a sub item
          {
            // Find the associated Item
            LVHITTESTINFO hi;
            hi.pt.x = pNm->ptAction.x;
            hi.pt.y = pNm->ptAction.y;
            hi.flags = LVHT_ONITEM ;
            SNDMSG(wnd.hWndList, LVM_SUBITEMHITTEST, 0, (LPARAM)&hi);
            ligne = hi.iItem;
          }

          if ( ligne >= 0)
          {
				ListView_GetItemText(	wnd.hWndList,
							      ligne,
                                        5,
                                        szbuff,
                                        MAX_PATH 
									);

				strcpy(LoadedGameFile, szbuff);
				emu.status = EMU_LOAD;


        break;
            }
            }

            break;
		}

		case WM_COMMAND:
		{
			switch( LOWORD(wParam) )
			{
			case ID_FILE_LOADDOL: WIN_OpenFile(); break;
			case ID_FILE_LOADCWMAP: HLE_Map_OpenFile(); break;
			case ID_FILE_CHOOSEROMDIRECTORY: BrowserSelectDir(); break;
			case ID_FILE_REFRESHROMLIST: BrowserReloadWindow(); break;
			case ID_EXIT: emu.status = EMU_QUIT; break;
			case ID_EMULATION_START:
				emu.started = true;

				if(cpu->loaded)
				{
					emu.status = EMU_RUNNING;

					if(cpu->IsCPUCompareActive() == 2)
					{
						PipeData = 'S';
						cpu->SendPipeData(&PipeData, 1);
						Sleep(500);
					}
				}
				else
					MessageBox(hWnd,
					  "Please load a ROM before starting the CPU!",
					  "Notice",NULL);
				break;
			case ID_EMULATION_STOP: 
				emu.started = false; 
				emu.status = EMU_HALTED; 
				GetWindowRect(wnd.hWnd, &rect);
				rect.bottom = rect.top + 480;
				rect.right = rect.left + 640;
				x = rect.left;
				y = rect.top;
				AdjustWindowRectEx(&rect, WS_CAPTION | WS_VISIBLE | WS_BORDER, TRUE, NULL);
				MoveWindow(wnd.hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, true);

				break;
			case ID_EMULATION_RESET: break;
			case ID_EMULATION_AUTOSTART: CFG_Enable_AutoStart(); break;
			case ID_SYSTEM_DYNAREC: CFG_Enable_DynaRec(); break;
			case ID_OPTIONS_ENABLEFRAMEBUFFER: CFG_Enable_Framebuffer(); break;
			case ID_OPTIONS_FULLSCREEN: CFG_Enable_Fullscreen(); break;
			case ID_OPTIONS_VIDEO_SETTINGS: CFG_Video_Open(); break;
			case ID_OPTIONS_JOYPAD_SETTINGS: CFG_Joypad_Open(); break;
			case ID_OPTIONS_DISABLEHLE:
				DisableHLEPatches = !DisableHLEPatches;
				if(DisableHLEPatches)
				{
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DISABLEHLE, MF_CHECKED);
					printf("Disabling HLE\n");
				}
				else
				{
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DISABLEHLE, MF_UNCHECKED);
					printf("Enabling HLE\n");
				}
				sprintf(szbuff, "%d", DisableHLEPatches);
				sprintf(str1, "%sGekko.ini", ProgramDirectory);
				WritePrivateProfileString("Debug", "Disable HLE Patches", szbuff, str1);
				break;

			case ID_OPTIONS_DISABLEINIPATCHES:
				DisableINIPatches = !DisableINIPatches;
				if(DisableINIPatches)
				{
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DISABLEINIPATCHES, MF_CHECKED);
					printf("Disabling INI Patches\n");
				}
				else
				{
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DISABLEINIPATCHES, MF_UNCHECKED);
					printf("Enabling INI Patches\n");
				}
				sprintf(szbuff, "%d", DisableINIPatches);
				sprintf(str1, "%sGekko.ini", ProgramDirectory);
				WritePrivateProfileString("Debug", "Disable INI Patches", szbuff, str1);
				break;

			case ID_DEBUGOPTIONS_DISPLAYFRAMERATE:
				DisplayFPS = !DisplayFPS;
				if(DisplayFPS)
					CheckMenuItem(GetMenu(hWnd), ID_DEBUGOPTIONS_DISPLAYFRAMERATE, MF_CHECKED);
				else
				{
					CheckMenuItem(GetMenu(hWnd), ID_DEBUGOPTIONS_DISPLAYFRAMERATE, MF_UNCHECKED);
					if(cpu->is_on)
					{
						char str1[100];
						switch(cpu->IsCPUCompareActive())
						{
							case 1:	//client
								sprintf(str1, APP_NAME " - Fucked Up");
								break;

							case 2: //server
								sprintf(str1, APP_NAME " - Gnarly");
								break;
							default:
								sprintf(str1, APP_NAME);
								break;
						}

						SetWindowText(hWnd, str1);
					}
				}

				sprintf(szbuff, "%d", DisplayFPS);
				sprintf(str1, "%sGekko.ini", ProgramDirectory);
				WritePrivateProfileString("System", "Display FPS", szbuff, str1);
				break;

			case ID_DEBUG_DUMPOPCODE0:
				cpu->DumpOp0 = !cpu->DumpOp0;
				if(cpu->DumpOp0)
				{
					CheckMenuItem(GetMenu(hWnd), ID_DEBUG_DUMPOPCODE0, MF_CHECKED);
					printf("Enabling Opcode 0 dump\n");
				}
				else
				{
					CheckMenuItem(GetMenu(hWnd), ID_DEBUG_DUMPOPCODE0, MF_UNCHECKED);
					printf("Disabling Opcode 0 dump\n");
				}
				sprintf(szbuff, "%d", cpu->DumpOp0);
				sprintf(str1, "%sGekko.ini", ProgramDirectory);
				WritePrivateProfileString("Debug", "Dump Opcode 0", szbuff, str1);
				break;

			case ID_DEBUG_PAUSEONUNKNOWNOPCODE:
				cpu->PauseOnUnknownOp = !cpu->PauseOnUnknownOp;
				if(cpu->PauseOnUnknownOp)
				{
					CheckMenuItem(GetMenu(hWnd), ID_DEBUG_PAUSEONUNKNOWNOPCODE, MF_CHECKED);
					printf("Enabling pause on unknown opcode\n");
				}
				else
				{
					CheckMenuItem(GetMenu(hWnd), ID_DEBUG_PAUSEONUNKNOWNOPCODE, MF_UNCHECKED);
					printf("Disabling pause on unknown opcode\n");
				}
				sprintf(szbuff, "%d", cpu->PauseOnUnknownOp);
				sprintf(str1, "%sGekko.ini", ProgramDirectory);
				WritePrivateProfileString("Debug", "Pause On Unknown Opcode", szbuff, str1);
				break;

			case ID_DEBUG_DEBUGGER:
				if(cpu->loaded)
					Debugger_Open();
				else
					MessageBox(hWnd,
					  "Please load a ROM before opening the Debugger!",
					  "Notice",NULL);
				break;

			case ID_DEBUG_DUMPRAM:
				{
					FILE * ramdump = fopen("c:\\ram.dump","wb");
					for (int i=0;i<RAM_SIZE;i++) {
						fwrite(Mem_RAM+i,1,1,ramdump);
					}
					fclose(ramdump);
					printf("ram dump complete\n");
					break;
				}
			case ID_DEBUGOPTIONS_DUMPBLOCKREADS:
				{
					DumpGCMBlockReads = !DumpGCMBlockReads;

					if(DumpGCMBlockReads)
					{
						CheckMenuItem(GetMenu(hWnd), ID_DEBUGOPTIONS_DUMPBLOCKREADS, MF_CHECKED);
						printf("Dumping GCM Reads\n");
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), ID_DEBUGOPTIONS_DUMPBLOCKREADS, MF_UNCHECKED);
						printf("Not Dumping GCM Reads\n");
					}
					sprintf(szbuff, "%d", DumpGCMBlockReads);
					sprintf(str1, "%sGekko.ini", ProgramDirectory);
					WritePrivateProfileString("Debug", "Dump GCM Reads", szbuff, str1);
					break;
				}

			case ID_COMPARECORES_STARTSERVER: cpu->StartPipe(0); break;
			case ID_COMPARECORES_STARTCLIENT: cpu->StartPipe(1); break;
			//case ID_DEBUG_OPTIONS_DUMPCRC: HLE_Map2Crc(); break;
			case ID_HELP_ABOUT: WIN_About_Open(); break;
			case ID_HELP_CONTENTS:
				sprintf(str1, "%s\\Help\\Gekko.chm", ProgramDirectory);
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.fMask = NULL;
				sei.hwnd = NULL;
				sei.lpVerb = "open";
				sei.lpFile = str1;
				sei.lpParameters= NULL;
				sei.nShow = SW_SHOWNORMAL;
				sei.hInstApp = NULL;
				sei.lpIDList = NULL;
				sei.lpClass = NULL;
				sei.hkeyClass = NULL;
				sei.dwHotKey = NULL;
				sei.hIcon = NULL;
				sei.hProcess = NULL;
				sei.lpDirectory = NULL;
				ReturnCode = ::ShellExecuteEx(&sei);
				break;
		
			/*case ID_HELP_GETLATESTGAMELIST:
				sprintf(str1, "http://nsider.gekko-emu.com/download.php/GameList");//, ProgramDirectory);
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.fMask = NULL;
				sei.hwnd = NULL;
				sei.lpVerb = "open";
				sei.lpFile = str1;
				sei.lpParameters= NULL;
				sei.nShow = SW_SHOWNORMAL;
				sei.hInstApp = NULL;
				sei.lpIDList = NULL;
				sei.lpClass = NULL;
				sei.hkeyClass = NULL;
				sei.dwHotKey = NULL;
				sei.hIcon = NULL;
				sei.hProcess = NULL;
				sei.lpDirectory = NULL;
				ReturnCode = ::ShellExecuteEx(&sei);
				break;

			case ID_HELP_HOMEPAGE:
				sprintf(str1, "http://www.gekko-emu.com/");//, ProgramDirectory);
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.fMask = NULL;
				sei.hwnd = NULL;
				sei.lpVerb = "open";
				sei.lpFile = str1;
				sei.lpParameters= NULL;
				sei.nShow = SW_SHOWNORMAL;
				sei.hInstApp = NULL;
				sei.lpIDList = NULL;
				sei.lpClass = NULL;
				sei.hkeyClass = NULL;
				sei.dwHotKey = NULL;
				sei.hIcon = NULL;
				sei.hProcess = NULL;
				sei.lpDirectory = NULL;
				ReturnCode = ::ShellExecuteEx(&sei);
				break;*/
			}

			return 0;
		}
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void WIN_Msg()
{
	if (PeekMessage(&wnd.msg,NULL,0,0,PM_REMOVE))
	{
		if (wnd.msg.message==WM_QUIT)
		{
			cpu->Halt();
			emu.status = EMU_QUIT;
		}
		else					
		{
			TranslateMessage(&wnd.msg);			
			DispatchMessage(&wnd.msg);			
		}
	}
	else								
	{
		if (emu.keys[VK_F1])		
		{
		}
	}
}

bool WIN_Create( void )
{
	wnd.active = true;

	WNDCLASS wc;

	wnd.hInstance = GetModuleHandle(0);
	
	wc.hInstance = wnd.hInstance;
	wc.lpfnWndProc = WIN_Proc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadImage(wnd.hInstance, MAKEINTRESOURCE(IDI_1GEKKO), IMAGE_ICON, 0, 0, 0);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.lpszClassName = "Gekko";
	wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	RegisterClass(&wc);

	wnd.hWnd = CreateWindow("Gekko", APP_NAME,
		WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		0, 0, 640, 480, NULL, NULL, wnd.hInstance, NULL);

	BrowserReloadWindow();

	return TRUE;
}
