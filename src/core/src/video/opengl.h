// opengl.h
// (c) 2005,2008 Gekko Team / Wiimu Project

#ifndef __OPENGL_H__
#define __OPENGL_H__

////////////////////////////////////////////////////////////////////////////////

// TODO: pcafe build broken...
//#define GLWIN_USE_SDL1
//#define GLWIN_USE_SDL2
#define GLWIN_USE_QT4

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define GP_NAME		"GL_Out"

////////////////////////////////////////////////////////////////////////////////

struct opengl
{
	u8 video_bpp;
	u32 video_width;
	u32 video_height;
	f32 video_width_scale;
	f32 video_height_scale;

	int	enable;
	int opened;
	int is_sleep;
	int wireframe;
	int fullscreen;

//	HGLRC hrc; 
//	HDC	 hdc;
};

extern opengl gl;

// TODO: EMU_FASTCALL shouldn't be necessary (gekko-qt workaround)
void EMU_FASTCALL OPENGL_SetTitle(char*);
void OPENGL_DrawFramebuffer();
void OPENGL_Initialize();
void OPENGL_Kill();
void OPENGL_Render();

#ifdef GLWIN_USE_QT4
class GRenderWindow;
void OPENGL_Create(GRenderWindow*);
#else
void OPENGL_Create();
#endif

////////////////////////////////////////////////////////////////////////////////
// EOF

#endif
