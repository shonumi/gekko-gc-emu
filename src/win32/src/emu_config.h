// emu_config.h
// (c) 2005,2008 Gekko Team

#ifndef _EMU_CONFIG_H_
#define _EMU_CONFIG_H_

////////////////////////////////////////////////////////////////////////////////

typedef struct t_videoConfig
{
	bool	wireframe;
	bool	force_alpha;
	bool	force_white;
	bool	dump_textures;
	bool	full_screen;
	bool	anistropic_filtering;
	bool	anti_aliasing;
	bool	efb_copy;

	int		shader_mode;
	int		window_res; 
	int		fullscreen_res; 
}videoConfig;

typedef struct t_joypadConfig
{
	int joypad_select;
	int joypad_device;

	struct sPad{
		bool	is_on;
		u8		analog[4],
				dpad[4],
				cstick[4],
				a, b, x, y, l, r, z, start;
	}pads[4];
}joypadConfig;

typedef struct t_emuConfig
{
	bool	enb_autostart;	
	bool	enb_framebuffer;	// unused
	bool	enb_systemcall;		// " "
}emuConfig;

////////////////////////////////////////////////////////////////////////////////

extern emuConfig cfg;
extern videoConfig gx_cfg;
extern videoConfig v_old;
extern joypadConfig jp_cfg;
extern joypadConfig jp_old;

////////////////////////////////////////////////////////////////////////////////

void CFG_Init(void);
void CFG_AutoSync(void);
void CFG_Enable_Framebuffer(void);
void CFG_Enable_AutoStart(void);
void CFG_Enable_SystemCall(void);
void CFG_Enable_DynaRec(void);
void CFG_Enable_Fullscreen(void);
void CFG_Video_Open(void);
void CFG_Video_Get_Ini(void);
void CFG_Video_Apply(void);
void CFG_Joypad_Open(void);
void CFG_Joypad_Get_Ini(void);
void CFG_Joypad_Apply(void);

// EOF

#endif
