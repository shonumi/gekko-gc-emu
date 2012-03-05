// emu_win.h
// (c) 2005,2006 Gekko Team

#ifndef _EMU_WIN_H_
#define _EMU_WIN_H_

////////////////////////////////////////////////////////////

typedef struct t_emuWnd
{
	HDC			hDC;
	HGLRC		hRC;

	HWND		hWnd;
	HWND		hWndStatus;
	HWND		hWndList;
	HWND		hWndDbg;

	HINSTANCE	hInstance;

	MSG			msg;

	bool		active;
	bool		fullscreen;
}emuWnd;

extern emuWnd dbg;
extern emuWnd wnd;
extern char g_szRomPath[1024];

////////////////////////////////////////////////////////////

bool				WIN_Create(void);
void				WIN_Msg();
LRESULT CALLBACK	WIN_Proc(HWND, UINT, WPARAM, LPARAM);
void				WIN_Framerate(void);
void				WIN_AddDebugLine(char *szBuf, int lineno);
void				WIN_ReloadWindow(void);
void				WIN_LoadFromString(char _filename[]);

//

#endif