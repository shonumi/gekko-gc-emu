// opengl.h
// (c) 2005,2008 Gekko Team / Wiimu Project

#ifndef __OPENGL_H__
#define __OPENGL_H__

////////////////////////////////////////////////////////////////////////////////


#include "SDL.h"
//#include "SDL_opengl.h"
#include "GL/glew.h"
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

void OPENGL_DrawFramebuffer();
void OPENGL_Initialize();
void OPENGL_Kill();
void OPENGL_Render();
void OPENGL_Create();

////////////////////////////////////////////////////////////////////////////////
// EOF

#endif
