// emu_config.cpp
// (c) 2005,2008 Gekko Team

#include "emu.h"
#include "video/gx_tev.h"
#include "hw/hw_vi.h"
#include "hw/hw_si.h"
#include "powerpc/cpu_core.h"
#include "powerpc/interpreter/cpu_int.h"
#include "powerpc/recompiler/cpu_rec.h"
#include "sdl.h"

emuConfig cfg;
videoConfig gx_cfg;
videoConfig gx_old;
joypadConfig jp_cfg;
joypadConfig jp_old;
char cfg_keyboard_text[256][256];
int waiting_for_input;
int button_pressed;
int destroy_joypad_config;

const int config_resolution_int[][2] =
{
	{640, 480},
	{800, 600},
	{1024, 768},
	{1152, 864},
	{1280, 800},
	{1280, 960},
	{1280, 1024},
	{1440, 900},
	{1600, 1200},
	{1680, 1050},
};

const char *config_resolution_text[10] = 
{ 
	"640x480",						// 0
	"800x600",						// 1
	"1024x768",						// 2
	"1152x864",						// 3
	"1280x800 (16:10)" ,			// 4
	"1280x960",						// 5
	"1280x1024",					// 6
	"1440x900 (16:10)" ,			// 7
	"1600x1200",					// 8
	"1680x1050 (16:10)",			// 9
};

const char *config_shaders_text[2] = 
{ 
	"No Shaders",					// 0
	"OpenGL 2.0 Shaders",			// 1
};

////////////////////////////////////////////////////////////////////////////////
// Emulator Configuration

void CFG_Init(void)
{
	int i;
	cpu = new GekkoCPUInterpreter();

	// Init variables
	cpu->is_sc = false; // toggle
	CFG_Enable_SystemCall(); // toggle back (updates menu)

	vi.is_autosync = false; // toggle
	CFG_AutoSync(); // toggle back (updates menu)

	// config/init video
	CFG_Video_Get_Ini(); // video config
	
	gx_cfg.full_screen = !gx_cfg.full_screen; // toggle
	CFG_Enable_Fullscreen(); // toggle back (updates menu)

	gl.video_width = config_resolution_int[gx_cfg.fullscreen_res][0];
	gl.video_height = config_resolution_int[gx_cfg.fullscreen_res][1];

	// config/init joypad
	jp_cfg.pads[0].is_on = true; // plug in controller 1
	CFG_Joypad_Get_Ini(); // joypad config
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK); // used for joypad and some video info

	// setup key text
    sprintf(cfg_keyboard_text[0], "undefined");
	for(i = '0'; i <= '9'; i++) sprintf(cfg_keyboard_text[i], "%c", i); // 0-9
	for(i = 'A'; i <= 'Z'; i++) sprintf(cfg_keyboard_text[i], "%c", i); // A-Z
	sprintf(cfg_keyboard_text[VK_LBUTTON], "LBUTTON");
	sprintf(cfg_keyboard_text[VK_RBUTTON], "RBUTTON");
	sprintf(cfg_keyboard_text[VK_CANCEL], "CANCEL");
	sprintf(cfg_keyboard_text[VK_MBUTTON], "MBUTTON");
	sprintf(cfg_keyboard_text[VK_BACK], "BACK");
	sprintf(cfg_keyboard_text[VK_TAB], "TAB");
	sprintf(cfg_keyboard_text[VK_CLEAR], "CLEAR");
	sprintf(cfg_keyboard_text[VK_RETURN], "RETURN");
	sprintf(cfg_keyboard_text[VK_SHIFT], "SHIFT");
	sprintf(cfg_keyboard_text[VK_CONTROL], "CONTROL");
	sprintf(cfg_keyboard_text[VK_MENU], "MENU");
	sprintf(cfg_keyboard_text[VK_PAUSE], "PAUSE");
	sprintf(cfg_keyboard_text[VK_CAPITAL], "CAPITAL");
	sprintf(cfg_keyboard_text[VK_KANA], "KANA");
	sprintf(cfg_keyboard_text[VK_JUNJA], "JUNJA");
	sprintf(cfg_keyboard_text[VK_FINAL], "FINAL");
	sprintf(cfg_keyboard_text[VK_HANJA], "HANJA");
	sprintf(cfg_keyboard_text[VK_ESCAPE], "ESCAPE");
	sprintf(cfg_keyboard_text[VK_CONVERT], "CONVERT");
	sprintf(cfg_keyboard_text[VK_NONCONVERT], "NONCONVERT");
	sprintf(cfg_keyboard_text[VK_ACCEPT], "ACCEPT");
	sprintf(cfg_keyboard_text[VK_MODECHANGE], "MODECHANGE");
	sprintf(cfg_keyboard_text[VK_SPACE], "SPACE");
	sprintf(cfg_keyboard_text[VK_PRIOR], "PRIOR");
	sprintf(cfg_keyboard_text[VK_NEXT], "NEXT");
	sprintf(cfg_keyboard_text[VK_END], "END");
	sprintf(cfg_keyboard_text[VK_HOME], "HOME");
	sprintf(cfg_keyboard_text[VK_LEFT], "LEFT");
	sprintf(cfg_keyboard_text[VK_UP], "UP");
	sprintf(cfg_keyboard_text[VK_RIGHT], "RIGHT");
	sprintf(cfg_keyboard_text[VK_DOWN], "DOWN");
	sprintf(cfg_keyboard_text[VK_SELECT], "SELECT");
	sprintf(cfg_keyboard_text[VK_PRINT], "PRINT");
	sprintf(cfg_keyboard_text[VK_EXECUTE], "EXECUTE");
	sprintf(cfg_keyboard_text[VK_SNAPSHOT], "SNAPSHOT");
	sprintf(cfg_keyboard_text[VK_INSERT], "INSERT");
	sprintf(cfg_keyboard_text[VK_DELETE], "DELETE");
	sprintf(cfg_keyboard_text[VK_HELP], "HELP");
	sprintf(cfg_keyboard_text[VK_LWIN], "LWIN");
	sprintf(cfg_keyboard_text[VK_RWIN], "RWIN");
	sprintf(cfg_keyboard_text[VK_APPS], "APPS");
	sprintf(cfg_keyboard_text[VK_SLEEP], "SLEEP");
	sprintf(cfg_keyboard_text[VK_NUMPAD0], "NUM0");
	sprintf(cfg_keyboard_text[VK_NUMPAD1], "NUM1");
	sprintf(cfg_keyboard_text[VK_NUMPAD2], "NUM2");
	sprintf(cfg_keyboard_text[VK_NUMPAD3], "NUM3");
	sprintf(cfg_keyboard_text[VK_NUMPAD4], "NUM4");
	sprintf(cfg_keyboard_text[VK_NUMPAD5], "NUM5");
	sprintf(cfg_keyboard_text[VK_NUMPAD6], "NUM6");
	sprintf(cfg_keyboard_text[VK_NUMPAD7], "NUM7");
	sprintf(cfg_keyboard_text[VK_NUMPAD8], "NUM8");
	sprintf(cfg_keyboard_text[VK_NUMPAD9], "NUM9");
	sprintf(cfg_keyboard_text[VK_MULTIPLY], "MULTIPLY");
	sprintf(cfg_keyboard_text[VK_ADD], "ADD");
	sprintf(cfg_keyboard_text[VK_SEPARATOR], "SEPARATOR");
	sprintf(cfg_keyboard_text[VK_SUBTRACT], "SUBTRACT");
	sprintf(cfg_keyboard_text[VK_DECIMAL], "DECIMAL");
	sprintf(cfg_keyboard_text[VK_DIVIDE], "DIVIDE");
	sprintf(cfg_keyboard_text[VK_F1], "F1");
	sprintf(cfg_keyboard_text[VK_F2], "F2");
	sprintf(cfg_keyboard_text[VK_F3], "F3");
	sprintf(cfg_keyboard_text[VK_F4], "F4");
	sprintf(cfg_keyboard_text[VK_F5], "F5");
	sprintf(cfg_keyboard_text[VK_F6], "F6");
	sprintf(cfg_keyboard_text[VK_F7], "F7");
	sprintf(cfg_keyboard_text[VK_F8], "F8");
	sprintf(cfg_keyboard_text[VK_F9], "F9");
	sprintf(cfg_keyboard_text[VK_F10], "F10");
	sprintf(cfg_keyboard_text[VK_F11], "F11");
	sprintf(cfg_keyboard_text[VK_F12], "F12");
	sprintf(cfg_keyboard_text[VK_F13], "F13");
	sprintf(cfg_keyboard_text[VK_F14], "F14");
	sprintf(cfg_keyboard_text[VK_F15], "F15");
	sprintf(cfg_keyboard_text[VK_F16], "F16");
	sprintf(cfg_keyboard_text[VK_F17], "F17");
	sprintf(cfg_keyboard_text[VK_F18], "F18");
	sprintf(cfg_keyboard_text[VK_F19], "F19");
	sprintf(cfg_keyboard_text[VK_F20], "F20");
	sprintf(cfg_keyboard_text[VK_F21], "F21");
	sprintf(cfg_keyboard_text[VK_F22], "F22");
	sprintf(cfg_keyboard_text[VK_F23], "F23");
	sprintf(cfg_keyboard_text[VK_F24], "F24");
	sprintf(cfg_keyboard_text[VK_NUMLOCK], "NUMLOCK");
	sprintf(cfg_keyboard_text[VK_SCROLL], "SCROLL");
	sprintf(cfg_keyboard_text[VK_LSHIFT], "LSHIFT");
	sprintf(cfg_keyboard_text[VK_RSHIFT], "RSHIFT");
	sprintf(cfg_keyboard_text[VK_LCONTROL], "LCONTROL");
	sprintf(cfg_keyboard_text[VK_RCONTROL], "RCONTROL");
	sprintf(cfg_keyboard_text[VK_LMENU], "LMENU");
	sprintf(cfg_keyboard_text[VK_RMENU], "RMENU");
	sprintf(cfg_keyboard_text[VK_OEM_1], "OEM1");
	sprintf(cfg_keyboard_text[VK_OEM_PLUS], "OEMPLUS");
	sprintf(cfg_keyboard_text[VK_OEM_COMMA], "OEMCOMMA");
	sprintf(cfg_keyboard_text[VK_OEM_MINUS], "OEMMINUS");
	sprintf(cfg_keyboard_text[VK_OEM_PERIOD], "OEMPERIOD");
	sprintf(cfg_keyboard_text[VK_OEM_2], "OEM2");
	sprintf(cfg_keyboard_text[VK_OEM_3], "OEM3");
	sprintf(cfg_keyboard_text[VK_OEM_4], "OEM4");
	sprintf(cfg_keyboard_text[VK_OEM_5], "OEM5");
	sprintf(cfg_keyboard_text[VK_OEM_6], "OEM6");
	sprintf(cfg_keyboard_text[VK_OEM_7], "OEM7");
	sprintf(cfg_keyboard_text[VK_OEM_8], "OEM8");
	sprintf(cfg_keyboard_text[VK_OEM_102], "OEM102");
	sprintf(cfg_keyboard_text[VK_PROCESSKEY], "PROCESS");
	sprintf(cfg_keyboard_text[VK_ATTN], "ATTN");
	sprintf(cfg_keyboard_text[VK_CRSEL], "CRSEL");
	sprintf(cfg_keyboard_text[VK_EXSEL], "EXSEL");
	sprintf(cfg_keyboard_text[VK_EREOF], "EREOF");
	sprintf(cfg_keyboard_text[VK_PLAY], "PLAY");
	sprintf(cfg_keyboard_text[VK_ZOOM], "ZOOM");
	sprintf(cfg_keyboard_text[VK_NONAME], "NONAME");
	sprintf(cfg_keyboard_text[VK_PA1], "PA1");
	sprintf(cfg_keyboard_text[VK_OEM_CLEAR], "OEM_CLEAR");
}

void CFG_AutoSync(void)
{
	/*int msg = MF_BYCOMMAND;
	if(vi.is_autosync)
	{
		vi.is_autosync = false;
		msg |= MFS_UNCHECKED;

	}else{
		vi.is_autosync = true;
		msg |= MFS_CHECKED;
	}

	CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_OPTIONS_AUTOSYNC,
					msg	);*/
}

void CFG_Enable_Fullscreen(void)
{
	int msg = MF_BYCOMMAND;
	char inifile[1024], szbuff[32];

	if(gx_cfg.full_screen)
	{
		gl.fullscreen = gx_cfg.full_screen = false; 
		msg |= MFS_UNCHECKED;

	}else{
		gl.fullscreen = gx_cfg.full_screen = true; 
		msg |= MFS_CHECKED;

	    // get fullscren resolution
	    gl.video_width = config_resolution_int[gx_cfg.fullscreen_res][0];
	    gl.video_height = config_resolution_int[gx_cfg.fullscreen_res][1];

		//if(emu.status == EMU_RUNNING)
		//	OPENGL_Create(wnd.hWnd);

		gl.video_width_scale = (f32)gl.video_width / 640.0f; // used for calculating viewports for higher res than gcn supports
		gl.video_height_scale = (f32)gl.video_height / 480.0f; // ..
	}

    // check/uncheck menu item off
	CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_OPTIONS_FULLSCREEN,
					msg	);	

    // update ini
	sprintf(inifile, "%sGekko.ini", ProgramDirectory);
	sprintf(szbuff, "%d", gx_cfg.full_screen);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"Fullscreen", (LPCWSTR)szbuff, (LPCWSTR)inifile);	
}

void CFG_Enable_Framebuffer(void)
{
	char szbuff[2], inifile[1024];
	int msg = MF_BYCOMMAND;
	
	// toggle
	if(cfg.enb_framebuffer)
	{
		cfg.enb_framebuffer = false;
		msg |= MFS_UNCHECKED;

	}
	else
	{
		cfg.enb_framebuffer = true;
		msg |= MFS_CHECKED;
	}

    // check/uncheck menu item off
	CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_OPTIONS_ENABLEFRAMEBUFFER,
					msg	);

	// write ini
	sprintf(inifile, "%sGekko.ini", ProgramDirectory);
	sprintf(szbuff, "%d", cfg.enb_framebuffer);
	WritePrivateProfileString((LPCWSTR)"System", (LPCWSTR)"Framebuffer", (LPCWSTR)szbuff, (LPCWSTR)inifile);
}


void CFG_Enable_AutoStart(void)
{
	char szbuff[2], inifile[1024];
	int msg = MF_BYCOMMAND;
	
	// toggle
  if(cfg.enb_autostart)
	{
		cfg.enb_autostart = false;
		msg |= MFS_UNCHECKED;
	}
	else
	{
		cfg.enb_autostart = true;
		msg |= MFS_CHECKED;
	}

    // check/uncheck menu item off
	CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_EMULATION_AUTOSTART,
					msg	);

	// write ini
	sprintf(inifile, "%sGekko.ini", ProgramDirectory);
	sprintf(szbuff, "%d", cfg.enb_autostart);
	WritePrivateProfileString((LPCWSTR)"Emulation", (LPCWSTR)"AutoStart", (LPCWSTR)szbuff, (LPCWSTR)inifile);
}

void CFG_Enable_SystemCall(void)
{
	/*int msg = MF_BYCOMMAND;
	if(cpu->is_sc)
	{
		cpu->is_sc = false;
		msg |= MFS_UNCHECKED;

	}else{
		cpu->is_sc = true;
		msg |= MFS_CHECKED;
	}

	CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_OPTIONS_ENABLESYSTEMCALL,
					msg	);*/
}

void CFG_Enable_DynaRec(void)
{
	char			szbuff[2];
	char			INIFile[1024];
	bool			PauseOnUnknownOp;
	bool			DumpOp0;

	int msg = MF_BYCOMMAND;
	bool is_sc;

	is_sc = cpu->is_sc;
	PauseOnUnknownOp = cpu->PauseOnUnknownOp;
	DumpOp0 = cpu->DumpOp0;
	if(cpu->GetCPUType() == GekkoCPU::DynaRec)
	{
		msg |= MFS_UNCHECKED;
		delete cpu;
		cpu = new GekkoCPUInterpreter();

		sprintf(szbuff, "%d", 0);
	}
	else
	{
		msg |= MFS_CHECKED;
		delete cpu;
		cpu = new GekkoCPURecompiler();

		sprintf(szbuff, "%d", 1);
	}

	sprintf(INIFile, "%sGekko.ini", ProgramDirectory);
	WritePrivateProfileString((LPCWSTR)"System", (LPCWSTR)"DynaRec", (LPCWSTR)szbuff, (LPCWSTR)INIFile);

	cpu->is_sc = is_sc;
	cpu->DumpOp0 = DumpOp0;
	cpu->PauseOnUnknownOp = PauseOnUnknownOp;
	CheckMenuItem(GetMenu(wnd.hWnd), ID_SYSTEM_DYNAREC, msg);
}

////////////////////////////////////////////////////////////////////////////////
// Video Configuration

void CFG_Video_Update_Ini(void)
{
	char inifile[1024], szbuff[32];

	sprintf(inifile, "%sGekko.ini", ProgramDirectory);

	sprintf(szbuff, "%d", gx_cfg.window_res);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"WindowRes", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.fullscreen_res);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"FullscreenRes", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.shader_mode);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"ShaderMode", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	
	sprintf(szbuff, "%d", gx_cfg.full_screen);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"Fullscreen", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.anti_aliasing);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"AntiAliasing", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.anistropic_filtering);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"AnistropicFiltering", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.wireframe);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"Wireframe", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.dump_textures);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"DumpTextures", (LPCWSTR)szbuff, (LPCWSTR)inifile);

	sprintf(szbuff, "%d", gx_cfg.force_white);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"ForceWhite", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	
	sprintf(szbuff, "%d", gx_cfg.force_alpha);
	WritePrivateProfileString((LPCWSTR)"Video", (LPCWSTR)"ForceAlpha", (LPCWSTR)szbuff, (LPCWSTR)inifile);
}

void CFG_Video_Get_Ini(void)
{
	char inifile[1024], szbuff[32];

	sprintf(inifile, "%sGekko.ini", ProgramDirectory);

	gx_cfg.window_res = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"WindowRes", 0, (LPCWSTR)inifile);
	gx_cfg.fullscreen_res = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"FullscreenRes", 0, (LPCWSTR)inifile);
	gx_cfg.shader_mode = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"ShaderMode", 0, (LPCWSTR)inifile);
	gx_cfg.full_screen = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"Fullscreen", 0, (LPCWSTR)inifile);
	gx_cfg.anti_aliasing = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"AntiAliasing", 0, (LPCWSTR)inifile);
	gx_cfg.anistropic_filtering = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"AnistropicFiltering", 0, (LPCWSTR)inifile);
	gx_cfg.wireframe = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"Wireframe", 0, (LPCWSTR)inifile);
	gx_cfg.dump_textures = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"DumpTextures", 0, (LPCWSTR)inifile);
	gx_cfg.force_white = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"ForceWhite", 0, (LPCWSTR)inifile);
	gx_cfg.force_alpha = GetPrivateProfileInt((LPCWSTR)"Video", (LPCWSTR)"ForceAlpha", 0, (LPCWSTR)inifile);
}

void CFG_Video_Apply(void)
{
	int x, y;
	RECT rect;

	// backup settings

	memcpy(&gx_old, &gx_cfg, sizeof(videoConfig));

	// wireframe

	gl.wireframe = gx_cfg.wireframe;

	if(!gx_cfg.wireframe)
	{
		if(gl.opened)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}else{
		if(gl.opened)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			
	}

	// window/fullscreen

	gl.fullscreen = gx_cfg.full_screen;
	if(gx_cfg.full_screen)
	{
		gl.video_width = config_resolution_int[gx_cfg.fullscreen_res][0];
		gl.video_height = config_resolution_int[gx_cfg.fullscreen_res][1];

		if(emu.status == EMU_RUNNING)
			OPENGL_Create(wnd.hWnd);

		CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_OPTIONS_FULLSCREEN,
					MF_BYCOMMAND | MFS_CHECKED	);	
	}else{
		gl.video_width = config_resolution_int[gx_cfg.window_res][0];
		gl.video_height = config_resolution_int[gx_cfg.window_res][1];

		CheckMenuItem(	GetMenu(wnd.hWnd),
					ID_OPTIONS_FULLSCREEN,
					MF_BYCOMMAND | MFS_UNCHECKED	);	
	}

	gl.video_width_scale = (f32)gl.video_width / 640.0f; // used for calculating viewports for higher res than gcn supports
	gl.video_height_scale = (f32)gl.video_height / 480.0f; // ..
	gl.fullscreen = gx_cfg.full_screen;

	if(emu.status == EMU_RUNNING && !gx_cfg.full_screen) // if in windowed, and already running, then just resize without reinitialization
	{ // better than fucking with a reinitializing gl
		GetWindowRect(wnd.hWnd, &rect);

		rect.bottom = rect.top + gl.video_height;
		rect.right = rect.left + gl.video_width;
		x = rect.left;
		y = rect.top;

		AdjustWindowRectEx(&rect, WS_CAPTION | WS_VISIBLE | WS_BORDER, TRUE, NULL);
		MoveWindow(wnd.hWnd, x, y, rect.right - rect.left, rect.bottom - rect.top, true);
	}
}

void CFG_Video_Save(HWND _hwnd)
{
	// force alpha
	gx_tev::destroy(); // recreate shaders

	// window/fullscreen resolution
	gx_cfg.window_res = SendDlgItemMessage(_hwnd, IDC_CONFIG_WINDOW_RESOLUTION, CB_GETCURSEL, 0, 0);
	gx_cfg.fullscreen_res = SendDlgItemMessage(_hwnd, IDC_CONFIG_FULLSCREEN_RESOLUTION, CB_GETCURSEL, 0, 0);

	// shader mode
	gx_cfg.shader_mode = SendDlgItemMessage(_hwnd, IDC_CONFIG_SHADER_MODE, CB_GETCURSEL, 0, 0);

	// apply changes
	CFG_Video_Apply();

	// update ini file
	CFG_Video_Update_Ini();
}

BOOL CALLBACK CFG_Video_Proc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
{
	char str1[100];

    switch(_umsg)
    {
        case WM_INITDIALOG:
			if(gx_cfg.wireframe)
				SendDlgItemMessage(_hwnd, IDC_CONFIG_WIREFRAME, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(_hwnd, IDC_CONFIG_WIREFRAME, BM_SETCHECK, BST_UNCHECKED, 0);

			if(gx_cfg.full_screen)
				SendDlgItemMessage(_hwnd, IDC_CONFIG_FULLSCREEN, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(_hwnd, IDC_CONFIG_FULLSCREEN, BM_SETCHECK, BST_UNCHECKED, 0);

			if(gx_cfg.force_alpha)
				SendDlgItemMessage(_hwnd, IDC_CONFIG_FORCE_ALPHA, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(_hwnd, IDC_CONFIG_FORCE_ALPHA, BM_SETCHECK, BST_UNCHECKED, 0);

			if(gx_cfg.dump_textures)
				SendDlgItemMessage(_hwnd, IDC_CONFIG_DUMP_TEXTURES, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(_hwnd, IDC_CONFIG_DUMP_TEXTURES, BM_SETCHECK, BST_UNCHECKED, 0);

			for(int i = 0; i < 10; i++)
			{
				SendDlgItemMessage(
					_hwnd, 
					IDC_CONFIG_WINDOW_RESOLUTION, 
					CB_ADDSTRING, 
					0, 
					reinterpret_cast<LPARAM>((LPCTSTR)config_resolution_text[i]));

				SendDlgItemMessage(
					_hwnd, 
					IDC_CONFIG_FULLSCREEN_RESOLUTION, 
					CB_ADDSTRING, 
					0, 
					reinterpret_cast<LPARAM>((LPCTSTR)config_resolution_text[i]));
			}

			SendDlgItemMessage(
				_hwnd, 
				IDC_CONFIG_SHADER_MODE, 
				CB_ADDSTRING, 
				0, 
				reinterpret_cast<LPARAM>((LPCTSTR)config_shaders_text[0]));

			SendDlgItemMessage(
				_hwnd, 
				IDC_CONFIG_SHADER_MODE, 
				CB_ADDSTRING, 
				0, 
				reinterpret_cast<LPARAM>((LPCTSTR)config_shaders_text[1]));
			

			SendDlgItemMessage(_hwnd, IDC_CONFIG_WINDOW_RESOLUTION, CB_SETCURSEL, gx_cfg.window_res, 0);
			SendDlgItemMessage(_hwnd, IDC_CONFIG_FULLSCREEN_RESOLUTION, CB_SETCURSEL, gx_cfg.fullscreen_res, 0);
			SendDlgItemMessage(_hwnd, IDC_CONFIG_SHADER_MODE, CB_SETCURSEL, gx_cfg.shader_mode, 0);

			sprintf(str1, "%s %s", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
			SetDlgItemText(_hwnd, IDC_VIDEO_DEVICE, (LPCWSTR)str1);

            return TRUE;

        case WM_CLOSE:
            EndDialog(_hwnd, 0);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(_wparam))
            {
                case IDC_CONFIG_OK:
                    CFG_Video_Save(_hwnd);
                    EndDialog(_hwnd, 0);
                    return TRUE;

                case IDC_CONFIG_CANCEL:
					memcpy(&gx_old, &gx_old, sizeof(videoConfig));
                    EndDialog(_hwnd, 0);
                    return TRUE;

				case IDC_CONFIG_APPLY:
					CFG_Video_Save(_hwnd);
					return TRUE;

				case IDC_CONFIG_WIREFRAME:
					if(gx_cfg.wireframe)
						gx_cfg.wireframe = false; 
					else
						gx_cfg.wireframe = true; 

					return TRUE;

				case IDC_CONFIG_FORCE_ALPHA:
					if(gx_cfg.force_alpha)
						gx_cfg.force_alpha = false; 
					else
						gx_cfg.force_alpha = true; 

					return TRUE;

				case IDC_CONFIG_DUMP_TEXTURES:
					if(gx_cfg.dump_textures)
						gx_cfg.dump_textures = false; 
					else
						gx_cfg.dump_textures = true; 

					return TRUE;

				case IDC_CONFIG_FULLSCREEN:
					if(gx_cfg.full_screen)
						gx_cfg.full_screen = false; 
					else
						gx_cfg.full_screen = true; 

					return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}

void CFG_Video_Open(void)
{
    DialogBox(
        wnd.hInstance,
        MAKEINTRESOURCE(IDD_VIDEO_CONFIG),
        wnd.hWnd,
        CFG_Video_Proc);
}

////////////////////////////////////////////////////////////////////////////////
// Joypad Configuration

void CFG_Joypad_Update_Ini(void)
{
	char inifile[1024], szbuff[32];

	sprintf(inifile, "%sGekko.ini", ProgramDirectory);

	sprintf(szbuff, "%d", jp_cfg.pads[0].is_on);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Enable", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].a);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0AKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].b);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0BKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].x);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0XKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].y);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0YKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].l);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0LKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].r);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0RKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].z);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0ZKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].start);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0StartKey", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].analog[0]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog0Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].analog[1]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog1Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].analog[2]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog2Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].analog[3]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog3Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].cstick[0]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick0Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].cstick[1]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick1Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].cstick[2]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick2Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].cstick[3]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick3Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].dpad[0]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad0Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].dpad[1]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad1Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].dpad[2]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad2Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[0].dpad[3]);
	WritePrivateProfileString((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad3Key", (LPCWSTR)szbuff, (LPCWSTR)inifile);

#pragma todo("Reimplement configs for joypads 1-3")
	/*sprintf(szbuff, "%d", jp_cfg.pads[1].is_on);
	WritePrivateProfileString("Joypad", "Pad1Enable", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].a);
	WritePrivateProfileString("Joypad", "Pad1AKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].b);
	WritePrivateProfileString("Joypad", "Pad1BKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].x);
	WritePrivateProfileString("Joypad", "Pad1XKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].y);
	WritePrivateProfileString("Joypad", "Pad1YKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].l);
	WritePrivateProfileString("Joypad", "Pad1LKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].r);
	WritePrivateProfileString("Joypad", "Pad1RKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].z);
	WritePrivateProfileString("Joypad", "Pad1ZKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].start);
	WritePrivateProfileString("Joypad", "Pad1StartKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].analog[0]);
	WritePrivateProfileString("Joypad", "Pad1Analog0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].analog[1]);
	WritePrivateProfileString("Joypad", "Pad1Analog1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].analog[2]);
	WritePrivateProfileString("Joypad", "Pad1Analog2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].analog[3]);
	WritePrivateProfileString("Joypad", "Pad1Analog3Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].cstick[0]);
	WritePrivateProfileString("Joypad", "Pad1CStick0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].cstick[1]);
	WritePrivateProfileString("Joypad", "Pad1CStick1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].cstick[2]);
	WritePrivateProfileString("Joypad", "Pad1CStick2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].cstick[3]);
	WritePrivateProfileString("Joypad", "Pad1CStick3Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].dpad[0]);
	WritePrivateProfileString("Joypad", "Pad1DPad0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].dpad[1]);
	WritePrivateProfileString("Joypad", "Pad1DPad1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].dpad[2]);
	WritePrivateProfileString("Joypad", "Pad1DPad2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[1].dpad[3]);
	WritePrivateProfileString("Joypad", "Pad1DPad3Key", szbuff, inifile);

	sprintf(szbuff, "%d", jp_cfg.pads[2].is_on);
	WritePrivateProfileString("Joypad", "Pad2Enable", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].a);
	WritePrivateProfileString("Joypad", "Pad2AKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].b);
	WritePrivateProfileString("Joypad", "Pad2BKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].x);
	WritePrivateProfileString("Joypad", "Pad2XKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].y);
	WritePrivateProfileString("Joypad", "Pad2YKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].l);
	WritePrivateProfileString("Joypad", "Pad2LKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].r);
	WritePrivateProfileString("Joypad", "Pad2RKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].z);
	WritePrivateProfileString("Joypad", "Pad2ZKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].start);
	WritePrivateProfileString("Joypad", "Pad2StartKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].analog[0]);
	WritePrivateProfileString("Joypad", "Pad2Analog0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].analog[1]);
	WritePrivateProfileString("Joypad", "Pad2Analog1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].analog[2]);
	WritePrivateProfileString("Joypad", "Pad2Analog2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].analog[3]);
	WritePrivateProfileString("Joypad", "Pad2Analog3Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].cstick[0]);
	WritePrivateProfileString("Joypad", "Pad2CStick0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].cstick[1]);
	WritePrivateProfileString("Joypad", "Pad2CStick1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].cstick[2]);
	WritePrivateProfileString("Joypad", "Pad2CStick2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].cstick[3]);
	WritePrivateProfileString("Joypad", "Pad2CStick3Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].dpad[0]);
	WritePrivateProfileString("Joypad", "Pad2DPad0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].dpad[1]);
	WritePrivateProfileString("Joypad", "Pad2DPad1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].dpad[2]);
	WritePrivateProfileString("Joypad", "Pad2DPad2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[2].dpad[3]);
	WritePrivateProfileString("Joypad", "Pad2DPad3Key", szbuff, inifile);

	sprintf(szbuff, "%d", jp_cfg.pads[3].is_on);
	WritePrivateProfileString("Joypad", "Pad3Enable", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].a);
	WritePrivateProfileString("Joypad", "Pad3AKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].b);
	WritePrivateProfileString("Joypad", "Pad3BKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].x);
	WritePrivateProfileString("Joypad", "Pad3XKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].y);
	WritePrivateProfileString("Joypad", "Pad3YKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].l);
	WritePrivateProfileString("Joypad", "Pad3LKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].r);
	WritePrivateProfileString("Joypad", "Pad3RKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].z);
	WritePrivateProfileString("Joypad", "Pad3ZKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].start);
	WritePrivateProfileString("Joypad", "Pad3StartKey", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].analog[0]);
	WritePrivateProfileString("Joypad", "Pad3Analog0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].analog[1]);
	WritePrivateProfileString("Joypad", "Pad3Analog1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].analog[2]);
	WritePrivateProfileString("Joypad", "Pad3Analog2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].analog[3]);
	WritePrivateProfileString("Joypad", "Pad3Analog3Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].cstick[0]);
	WritePrivateProfileString("Joypad", "Pad3CStick0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].cstick[1]);
	WritePrivateProfileString("Joypad", "Pad3CStick1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].cstick[2]);
	WritePrivateProfileString("Joypad", "Pad3CStick2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].cstick[3]);
	WritePrivateProfileString("Joypad", "Pad3CStick3Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].dpad[0]);
	WritePrivateProfileString("Joypad", "Pad3DPad0Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].dpad[1]);
	WritePrivateProfileString("Joypad", "Pad3DPad1Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].dpad[2]);
	WritePrivateProfileString("Joypad", "Pad3DPad2Key", szbuff, inifile);
	sprintf(szbuff, "%d", jp_cfg.pads[3].dpad[3]);
	WritePrivateProfileString("Joypad", "Pad3DPad3Key", szbuff, inifile);*/
}

void CFG_Joypad_Get_Ini(void)
{
	char inifile[1024], szbuff[32];

	sprintf(inifile, "%sGekko.ini", ProgramDirectory);

	jp_cfg.pads[0].is_on = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Enable", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].a = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0AKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].b = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0BKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].x = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0XKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].y = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0YKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].l = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0LKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].r = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0RKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].z = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0ZKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].start = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0StartKey", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].analog[0] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog0Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].analog[1] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog1Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].analog[2] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog2Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].analog[3] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0Analog3Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].cstick[0] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick0Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].cstick[1] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick1Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].cstick[2] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick2Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].cstick[3] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0CStick3Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].dpad[0] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad0Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].dpad[1] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad1Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].dpad[2] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad2Key", 0, (LPCWSTR)inifile);
	jp_cfg.pads[0].dpad[3] = GetPrivateProfileInt((LPCWSTR)"Joypad", (LPCWSTR)"Pad0DPad3Key", 0, (LPCWSTR)inifile);

	/*jp_cfg.pads[1].is_on = GetPrivateProfileInt("Joypad", "Pad1Enable", 0, inifile);
	jp_cfg.pads[1].a = GetPrivateProfileInt("Joypad", "Pad1AKey", 0, inifile);
	jp_cfg.pads[1].b = GetPrivateProfileInt("Joypad", "Pad1BKey", 0, inifile);
	jp_cfg.pads[1].x = GetPrivateProfileInt("Joypad", "Pad1XKey", 0, inifile);
	jp_cfg.pads[1].y = GetPrivateProfileInt("Joypad", "Pad1YKey", 0, inifile);
	jp_cfg.pads[1].l = GetPrivateProfileInt("Joypad", "Pad1LKey", 0, inifile);
	jp_cfg.pads[1].r = GetPrivateProfileInt("Joypad", "Pad1RKey", 0, inifile);
	jp_cfg.pads[1].z = GetPrivateProfileInt("Joypad", "Pad1ZKey", 0, inifile);
	jp_cfg.pads[1].start = GetPrivateProfileInt("Joypad", "Pad1StartKey", 0, inifile);
	jp_cfg.pads[1].analog[0] = GetPrivateProfileInt("Joypad", "Pad1Analog0Key", 0, inifile);
	jp_cfg.pads[1].analog[1] = GetPrivateProfileInt("Joypad", "Pad1Analog1Key", 0, inifile);
	jp_cfg.pads[1].analog[2] = GetPrivateProfileInt("Joypad", "Pad1Analog2Key", 0, inifile);
	jp_cfg.pads[1].analog[3] = GetPrivateProfileInt("Joypad", "Pad1Analog3Key", 0, inifile);
	jp_cfg.pads[1].cstick[0] = GetPrivateProfileInt("Joypad", "Pad1CStick0Key", 0, inifile);
	jp_cfg.pads[1].cstick[1] = GetPrivateProfileInt("Joypad", "Pad1CStick1Key", 0, inifile);
	jp_cfg.pads[1].cstick[2] = GetPrivateProfileInt("Joypad", "Pad1CStick2Key", 0, inifile);
	jp_cfg.pads[1].cstick[3] = GetPrivateProfileInt("Joypad", "Pad1CStick3Key", 0, inifile);
	jp_cfg.pads[1].dpad[0] = GetPrivateProfileInt("Joypad", "Pad1DPad0Key", 0, inifile);
	jp_cfg.pads[1].dpad[1] = GetPrivateProfileInt("Joypad", "Pad1DPad1Key", 0, inifile);
	jp_cfg.pads[1].dpad[2] = GetPrivateProfileInt("Joypad", "Pad1DPad2Key", 0, inifile);
	jp_cfg.pads[1].dpad[3] = GetPrivateProfileInt("Joypad", "Pad1DPad3Key", 0, inifile);

	jp_cfg.pads[2].is_on = GetPrivateProfileInt("Joypad", "Pad2Enable", 0, inifile);
	jp_cfg.pads[2].a = GetPrivateProfileInt("Joypad", "Pad2AKey", 0, inifile);
	jp_cfg.pads[2].b = GetPrivateProfileInt("Joypad", "Pad2BKey", 0, inifile);
	jp_cfg.pads[2].x = GetPrivateProfileInt("Joypad", "Pad2XKey", 0, inifile);
	jp_cfg.pads[2].y = GetPrivateProfileInt("Joypad", "Pad2YKey", 0, inifile);
	jp_cfg.pads[2].l = GetPrivateProfileInt("Joypad", "Pad2LKey", 0, inifile);
	jp_cfg.pads[2].r = GetPrivateProfileInt("Joypad", "Pad2RKey", 0, inifile);
	jp_cfg.pads[2].z = GetPrivateProfileInt("Joypad", "Pad2ZKey", 0, inifile);
	jp_cfg.pads[2].start = GetPrivateProfileInt("Joypad", "Pad2StartKey", 0, inifile);
	jp_cfg.pads[2].analog[0] = GetPrivateProfileInt("Joypad", "Pad2Analog0Key", 0, inifile);
	jp_cfg.pads[2].analog[1] = GetPrivateProfileInt("Joypad", "Pad2Analog1Key", 0, inifile);
	jp_cfg.pads[2].analog[2] = GetPrivateProfileInt("Joypad", "Pad2Analog2Key", 0, inifile);
	jp_cfg.pads[2].analog[3] = GetPrivateProfileInt("Joypad", "Pad2Analog3Key", 0, inifile);
	jp_cfg.pads[2].cstick[0] = GetPrivateProfileInt("Joypad", "Pad2CStick0Key", 0, inifile);
	jp_cfg.pads[2].cstick[1] = GetPrivateProfileInt("Joypad", "Pad2CStick1Key", 0, inifile);
	jp_cfg.pads[2].cstick[2] = GetPrivateProfileInt("Joypad", "Pad2CStick2Key", 0, inifile);
	jp_cfg.pads[2].cstick[3] = GetPrivateProfileInt("Joypad", "Pad2CStick3Key", 0, inifile);
	jp_cfg.pads[2].dpad[0] = GetPrivateProfileInt("Joypad", "Pad2DPad0Key", 0, inifile);
	jp_cfg.pads[2].dpad[1] = GetPrivateProfileInt("Joypad", "Pad2DPad1Key", 0, inifile);
	jp_cfg.pads[2].dpad[2] = GetPrivateProfileInt("Joypad", "Pad2DPad2Key", 0, inifile);
	jp_cfg.pads[2].dpad[3] = GetPrivateProfileInt("Joypad", "Pad2DPad3Key", 0, inifile);

	jp_cfg.pads[3].is_on = GetPrivateProfileInt("Joypad", "Pad3Enable", 0, inifile);
	jp_cfg.pads[3].a = GetPrivateProfileInt("Joypad", "Pad3AKey", 0, inifile);
	jp_cfg.pads[3].b = GetPrivateProfileInt("Joypad", "Pad3BKey", 0, inifile);
	jp_cfg.pads[3].x = GetPrivateProfileInt("Joypad", "Pad3XKey", 0, inifile);
	jp_cfg.pads[3].y = GetPrivateProfileInt("Joypad", "Pad3YKey", 0, inifile);
	jp_cfg.pads[3].l = GetPrivateProfileInt("Joypad", "Pad3LKey", 0, inifile);
	jp_cfg.pads[3].r = GetPrivateProfileInt("Joypad", "Pad3RKey", 0, inifile);
	jp_cfg.pads[3].z = GetPrivateProfileInt("Joypad", "Pad3ZKey", 0, inifile);
	jp_cfg.pads[3].start = GetPrivateProfileInt("Joypad", "Pad3StartKey", 0, inifile);
	jp_cfg.pads[3].analog[0] = GetPrivateProfileInt("Joypad", "Pad3Analog0Key", 0, inifile);
	jp_cfg.pads[3].analog[1] = GetPrivateProfileInt("Joypad", "Pad3Analog1Key", 0, inifile);
	jp_cfg.pads[3].analog[2] = GetPrivateProfileInt("Joypad", "Pad3Analog2Key", 0, inifile);
	jp_cfg.pads[3].analog[3] = GetPrivateProfileInt("Joypad", "Pad3Analog3Key", 0, inifile);
	jp_cfg.pads[3].cstick[0] = GetPrivateProfileInt("Joypad", "Pad3CStick0Key", 0, inifile);
	jp_cfg.pads[3].cstick[1] = GetPrivateProfileInt("Joypad", "Pad3CStick1Key", 0, inifile);
	jp_cfg.pads[3].cstick[2] = GetPrivateProfileInt("Joypad", "Pad3CStick2Key", 0, inifile);
	jp_cfg.pads[3].cstick[3] = GetPrivateProfileInt("Joypad", "Pad3CStick3Key", 0, inifile);
	jp_cfg.pads[3].dpad[0] = GetPrivateProfileInt("Joypad", "Pad3DPad0Key", 0, inifile);
	jp_cfg.pads[3].dpad[1] = GetPrivateProfileInt("Joypad", "Pad3DPad1Key", 0, inifile);
	jp_cfg.pads[3].dpad[2] = GetPrivateProfileInt("Joypad", "Pad3DPad2Key", 0, inifile);
	jp_cfg.pads[3].dpad[3] = GetPrivateProfileInt("Joypad", "Pad3DPad3Key", 0, inifile);*/
}

void CFG_Joypad_Save(HWND _hwnd)
{
    // enabled/disable pads
    for(int i = 0; i < 4; i++)
	{
        si.pad_connected[i] = jp_cfg.pads[i].is_on;
	}
    
	// update ini file
	CFG_Joypad_Update_Ini();
}

void CFG_Joypad_Update_Text(HWND _hwnd)
{
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_START, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].start]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_A, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].a]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_B, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].b]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_X, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].x]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_Y, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].y]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_L, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].l]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_R, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].r]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_Z, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].z]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_ANALOGUP, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].analog[0]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_ANALOGDOWN, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].analog[1]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_ANALOGLEFT, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].analog[2]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_ANALOGRIGHT, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].analog[3]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_CUP, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].cstick[0]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_CDOWN, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].cstick[1]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_CLEFT, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].cstick[2]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_CRIGHT, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].cstick[3]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_DUP, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].dpad[0]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_DDOWN, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].dpad[1]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_DLEFT, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].dpad[2]]);
	SetDlgItemText(_hwnd, IDC_JOYPAD_STATIC_DRIGHT, (LPCWSTR)cfg_keyboard_text[jp_cfg.pads[jp_cfg.joypad_select].dpad[3]]);
}

BOOL CALLBACK CFG_Joypad_Proc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam)
{
    switch(_umsg)
    {
        case WM_INITDIALOG:
			char str1[256];
			sprintf(str1, "Configure Joypad %d", jp_cfg.joypad_select);
			SetDlgItemText(_hwnd, IDC_JOYPAD_GROUP, (LPCWSTR)str1);

			if(jp_cfg.pads[jp_cfg.joypad_select].is_on)
				SendDlgItemMessage(_hwnd, IDC_JOYPAD_PLUGGED, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(_hwnd, IDC_JOYPAD_PLUGGED, BM_SETCHECK, BST_UNCHECKED, 0);
				
			CFG_Joypad_Update_Text(_hwnd);

			memcpy(&jp_old, &jp_cfg, sizeof(joypadConfig));

            return TRUE;

        case WM_CLOSE:
            EndDialog(_hwnd, 0);
			destroy_joypad_config = 1;
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(_wparam))
            {
                case IDC_JOYPAD_OK:
                    CFG_Joypad_Save(_hwnd);
                    EndDialog(_hwnd, 0);
					waiting_for_input = button_pressed = 0;
					destroy_joypad_config = 1;
                    return TRUE;
                    
				case IDC_JOYPAD_APPLY:
					CFG_Joypad_Save(_hwnd);
					return TRUE;

                case IDC_JOYPAD_CANCEL:
					memcpy(&jp_cfg, &jp_old, sizeof(joypadConfig));
                    EndDialog(_hwnd, 0);
					waiting_for_input = button_pressed = 0;
					destroy_joypad_config = 1;
                    return TRUE;
                    
                case IDC_JOYPAD_DEFAULT:
					jp_cfg.pads[jp_cfg.joypad_select].analog[0] = 87; // WASD
					jp_cfg.pads[jp_cfg.joypad_select].analog[1] = 83; 
					jp_cfg.pads[jp_cfg.joypad_select].analog[2] = 65; 
					jp_cfg.pads[jp_cfg.joypad_select].analog[3] = 68; 
                    jp_cfg.pads[jp_cfg.joypad_select].dpad[0] = 49; // 1,2,3,4
					jp_cfg.pads[jp_cfg.joypad_select].dpad[1] = 50; 
					jp_cfg.pads[jp_cfg.joypad_select].dpad[2] = 52; 
					jp_cfg.pads[jp_cfg.joypad_select].dpad[3] = 51; 
                    jp_cfg.pads[jp_cfg.joypad_select].cstick[0] = 38; // Arrows
					jp_cfg.pads[jp_cfg.joypad_select].cstick[1] = 40;
					jp_cfg.pads[jp_cfg.joypad_select].cstick[2] = 37;
					jp_cfg.pads[jp_cfg.joypad_select].cstick[3] = 39;
                    jp_cfg.pads[jp_cfg.joypad_select].a = 16; // Shift
                    jp_cfg.pads[jp_cfg.joypad_select].b = 32; // Space
                    jp_cfg.pads[jp_cfg.joypad_select].x = 81; // Q
                    jp_cfg.pads[jp_cfg.joypad_select].y = 69; // Y
                    jp_cfg.pads[jp_cfg.joypad_select].l = 9; // Tab
                    jp_cfg.pads[jp_cfg.joypad_select].r = 82; // R
                    jp_cfg.pads[jp_cfg.joypad_select].z = 70; // F
                    jp_cfg.pads[jp_cfg.joypad_select].start = 13; // Enter
                    CFG_Joypad_Update_Text(_hwnd);
                    return TRUE;

                case IDC_JOYPAD_CLEAR:
                    jp_cfg.pads[jp_cfg.joypad_select].analog[0] = 
					jp_cfg.pads[jp_cfg.joypad_select].analog[1] = 
					jp_cfg.pads[jp_cfg.joypad_select].analog[2] = 
					jp_cfg.pads[jp_cfg.joypad_select].analog[3] = 
                    jp_cfg.pads[jp_cfg.joypad_select].dpad[0] = 
					jp_cfg.pads[jp_cfg.joypad_select].dpad[1] = 
					jp_cfg.pads[jp_cfg.joypad_select].dpad[2] = 
					jp_cfg.pads[jp_cfg.joypad_select].dpad[3] = 
                    jp_cfg.pads[jp_cfg.joypad_select].cstick[0] = 
					jp_cfg.pads[jp_cfg.joypad_select].cstick[1] = 
					jp_cfg.pads[jp_cfg.joypad_select].cstick[2] = 
					jp_cfg.pads[jp_cfg.joypad_select].cstick[3] = 
                    jp_cfg.pads[jp_cfg.joypad_select].a = 
                    jp_cfg.pads[jp_cfg.joypad_select].b = 
                    jp_cfg.pads[jp_cfg.joypad_select].x =
                    jp_cfg.pads[jp_cfg.joypad_select].y = 
                    jp_cfg.pads[jp_cfg.joypad_select].l = 
                    jp_cfg.pads[jp_cfg.joypad_select].r =
                    jp_cfg.pads[jp_cfg.joypad_select].z = 
                    jp_cfg.pads[jp_cfg.joypad_select].start = 0;
                    CFG_Joypad_Update_Text(_hwnd);
                    return TRUE;
                    
				case IDC_JOYPAD_A:
				case IDC_JOYPAD_B:
				case IDC_JOYPAD_X:
				case IDC_JOYPAD_Y:
				case IDC_JOYPAD_L:
				case IDC_JOYPAD_R:
				case IDC_JOYPAD_Z:
				case IDC_JOYPAD_START:
				case IDC_JOYPAD_ANALOGUP:
				case IDC_JOYPAD_ANALOGDOWN:
				case IDC_JOYPAD_ANALOGLEFT:
				case IDC_JOYPAD_ANALOGRIGHT:
				case IDC_JOYPAD_DUP:
				case IDC_JOYPAD_DDOWN:
				case IDC_JOYPAD_DLEFT:
				case IDC_JOYPAD_DRIGHT:
				case IDC_JOYPAD_CUP:
				case IDC_JOYPAD_CDOWN:
				case IDC_JOYPAD_CLEFT:
				case IDC_JOYPAD_CRIGHT:
					waiting_for_input = 1;
					button_pressed = LOWORD(_wparam);
					SetWindowText(_hwnd, (LPCWSTR)"Joypad Settings (Waiting for key press...)");
					
					return TRUE;

				case IDC_JOYPAD_PLUGGED:
					if(jp_cfg.pads[jp_cfg.joypad_select].is_on)
						jp_cfg.pads[jp_cfg.joypad_select].is_on = false; 
					else
						jp_cfg.pads[jp_cfg.joypad_select].is_on = true; 

					return TRUE;

				case IDC_JOYPAD_CONTROLLER1:
				case IDC_JOYPAD_CONTROLLER2:
				case IDC_JOYPAD_CONTROLLER3:
				case IDC_JOYPAD_CONTROLLER4:

					char str1[256];
					jp_cfg.joypad_select = LOWORD(_wparam) - IDC_JOYPAD_CONTROLLER1;
					sprintf(str1, "Configure Joypad %d", jp_cfg.joypad_select);
					SetDlgItemText(_hwnd, IDC_JOYPAD_GROUP, (LPCWSTR)str1);

					if(jp_cfg.pads[jp_cfg.joypad_select].is_on)
						SendDlgItemMessage(_hwnd, IDC_JOYPAD_PLUGGED, BM_SETCHECK, BST_CHECKED, 0);
					else
						SendDlgItemMessage(_hwnd, IDC_JOYPAD_PLUGGED, BM_SETCHECK, BST_UNCHECKED, 0);

					CFG_Joypad_Update_Text(_hwnd);
					
					return TRUE;
            }
            break;

        default:
            return FALSE;
    }
    return FALSE;
}

void CFG_Joypad_Open(void)
{
	destroy_joypad_config = 0;

    HWND hWnd = CreateDialog(
        wnd.hInstance,
        MAKEINTRESOURCE(IDD_JOYPAD_CONFIG),
        wnd.hWnd,
        CFG_Joypad_Proc);

	ShowWindow(hWnd, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if(destroy_joypad_config) return;
		if (msg.message == WM_KEYDOWN && waiting_for_input)
		{
			if (msg.wParam == VK_ESCAPE)
				msg.wParam = 0;

			switch(button_pressed)
			{
				case IDC_JOYPAD_A:
					jp_cfg.pads[jp_cfg.joypad_select].a = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_A, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_B:
					jp_cfg.pads[jp_cfg.joypad_select].b = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_B, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_X:
					jp_cfg.pads[jp_cfg.joypad_select].x = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_X, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_Y:
					jp_cfg.pads[jp_cfg.joypad_select].y = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_Y, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_L:
					jp_cfg.pads[jp_cfg.joypad_select].l = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_L, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_R:
					jp_cfg.pads[jp_cfg.joypad_select].r = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_R, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_Z:
					jp_cfg.pads[jp_cfg.joypad_select].z = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_Z, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_START:
					jp_cfg.pads[jp_cfg.joypad_select].start = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_START, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_ANALOGUP:
					jp_cfg.pads[jp_cfg.joypad_select].analog[0] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_ANALOGUP, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_ANALOGDOWN:
					jp_cfg.pads[jp_cfg.joypad_select].analog[1] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_ANALOGDOWN, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_ANALOGLEFT:
					jp_cfg.pads[jp_cfg.joypad_select].analog[2] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_ANALOGLEFT, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_ANALOGRIGHT:
					jp_cfg.pads[jp_cfg.joypad_select].analog[3] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_ANALOGRIGHT, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_DUP:
					jp_cfg.pads[jp_cfg.joypad_select].dpad[0] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_DUP, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_DDOWN:
					jp_cfg.pads[jp_cfg.joypad_select].dpad[1] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_DDOWN, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_DLEFT:
					jp_cfg.pads[jp_cfg.joypad_select].dpad[2] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_DLEFT, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_DRIGHT:
					jp_cfg.pads[jp_cfg.joypad_select].dpad[3] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_DRIGHT, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_CUP:
					jp_cfg.pads[jp_cfg.joypad_select].cstick[0] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_CUP, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_CDOWN:
					jp_cfg.pads[jp_cfg.joypad_select].cstick[1] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_CDOWN, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_CLEFT:
					jp_cfg.pads[jp_cfg.joypad_select].cstick[2] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_CLEFT, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
				case IDC_JOYPAD_CRIGHT:
					jp_cfg.pads[jp_cfg.joypad_select].cstick[3] = msg.wParam;
					SetDlgItemText(hWnd, IDC_JOYPAD_STATIC_CRIGHT, (LPCWSTR)cfg_keyboard_text[msg.wParam]);
					break;
			}
			waiting_for_input = 0;
			SetWindowText(hWnd, (LPCWSTR)"Joypad Settings");
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

////////////////////////////////////////////////////////////////////////////////
