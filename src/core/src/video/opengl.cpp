// TODO: Should probably move this file to GUI frontends
// opengl.cpp
// (c) 2005,2008 Gekko Team / Wiimu Project

#include "common.h"

#include "input_common.h"

#include "memory.h"
#include "powerpc/cpu_core.h"
#include "dvd/gcm.h"
#include "opengl.h"
#include "gx_fifo.h"
#include "hw/hw_vi.h"
#include "gx_tev.h"
#include "gx_texture.h"
#include "emuwindow.h"

opengl gl;
static EmuWindow* s_render_window = NULL;

////////////////////////////////////////////////////////////////////////////////

char g_window_title[256];

void OPENGL_SetWindow(EmuWindow* render_window)
{
    s_render_window = render_window;
}

// Set the window title
void OPENGL_SetTitle(char* title) {
    sprintf_s(g_window_title, 256, "%s", title); 
}

void OPENGL_DrawFramebuffer()
{
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640,480, GL_RGBA, GL_UNSIGNED_BYTE, vi.fb_data);

	glBindTexture(GL_TEXTURE_2D, vi.fb_texture); 

	const static float tU = (gl.video_width / 1024.f);
	const static float tV = (gl.video_height / 512.f);
	
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);	
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.f,tV); 	glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(tU, tV); 	glVertex3f(1.0f, -1.0f,  1.0f);	
		glTexCoord2f(tU, 0.f);	glVertex3f(1.0f,  1.0,  1.0f);	
		glTexCoord2f(0.f,0.f); 	glVertex3f(-1.0f,  1.0f,  1.0f);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
}

void OPENGL_Initialize()
{
    s_render_window->MakeCurrent();
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CW);

	// Wireframe
	if(gl.wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			
	}else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Framebuffer
#pragma todo("renable framebuffer support")
/*    if(cfg.enb_framebuffer)	
    {						
    	glShadeModel(GL_SMOOTH);							
    	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				
    	glClearDepth(1.0f);									
    
    	glEnable(GL_DEPTH_TEST);							
    	glDepthFunc(GL_LEQUAL);	
    	
    	glGenTextures(1, &vi.fb_texture);
    	glBindTexture(GL_TEXTURE_2D, vi.fb_texture);
    	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, vi.fb_data);
    }*/

	// backup settings

	//memcpy(&gx_old, &gx_cfg, sizeof(videoConfig));

	// wireframe

	gl.wireframe = 0; //gx_cfg.wireframe;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//if(!gx_cfg.wireframe)
	//{
	//	if(gl.opened)
	//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//}else{
	//	if(gl.opened)
	//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			
	//}

	// window/fullscreen

	gl.fullscreen = 0;
	if(0)
	{
		//gl.video_width = config_resolution_int[gx_cfg.fullscreen_res][0];
		//gl.video_height = config_resolution_int[gx_cfg.fullscreen_res][1];

		//if(emu.status == EMU_RUNNING)
		//	OPENGL_Create(wnd.hWnd);

		//CheckMenuItem(	GetMenu(wnd.hWnd),
		//			ID_OPTIONS_FULLSCREEN,
		//			MF_BYCOMMAND | MFS_CHECKED	);	
	}else{
		gl.video_width = 640;//config_resolution_int[gx_cfg.window_res][0];
		gl.video_height = 480;//config_resolution_int[gx_cfg.window_res][1];

		//CheckMenuItem(	GetMenu(wnd.hWnd),
		//			ID_OPTIONS_FULLSCREEN,
		//			MF_BYCOMMAND | MFS_UNCHECKED	);	
	}

	gl.video_width_scale = (f32)gl.video_width / 640.0f; // used for calculating viewports for higher res than gcn supports
	gl.video_height_scale = (f32)gl.video_height / 480.0f; // ..
	//gl.fullscreen = gx_cfg.full_screen;

}

void OPENGL_Kill()
{
	/*if (gl.fullscreen)										
	{
		ChangeDisplaySettings(NULL,0); // switch back to desktop
		ShowCursor(TRUE); // show mouse pointer
	}

	if(gl.hrc)											
	{
		if (!wglMakeCurrent(NULL, NULL))					
			printf("OPENGL Release Of DC And RC Failed.");

		if (!wglDeleteContext(gl.hrc))						
			printf("OPENGL Release Rendering Context Failed.");

		gl.hrc = NULL;										
	}

	if(gl.hdc && !ReleaseDC(_hwnd, gl.hdc))				
	{
		printf("OPENGL Release Device Context Failed.");
		gl.hdc = NULL;										
	}*/

    gl.opened = false;
}

void OPENGL_SetTitle()
{   
	static u32 swaps = 0, last = 0;
	static float fps = 0;
	u32 t = SDL_GetTicks();
	static u64 opcount = 0;
	s64 ops = 0;
	f32 mips = 0;
	static double opsspeed = 0;
	char str1[255];
	char str2[15];
	swaps++;
		
	if(t - last > 1000) {
		fps = (float) swaps / (0.001 * (t - last));
		ops = (cpu->GetTicks() - opcount);
		opsspeed = (((s64)ops) / (float)(cpu->GetTicksPerSecond()));
		mips = ((float)ops)/1000000;
		swaps = 0;
		last = t;
		opcount = cpu->GetTicks();

		if (cpu->GetCPUType() == GekkoCPU::DynaRec) {
			sprintf_s(str2, 15, "rec");
        } else {
            sprintf_s(str2, 15, "int");
		}
		sprintf_s(str1, 255, "%s (%s) - %03.02f fps - %03.02f mips (%03.02f%%) - %s", 
            g_window_title, str2, fps, mips, opsspeed*10, dvd::g_current_game_name);

        s_render_window->SetTitle(str1);
	}
}

void OPENGL_Render()
{
    glFlush();
    s_render_window->SwapBuffers();

    input_common::g_user_input->PollEvent();
    OPENGL_SetTitle();
}

void OPENGL_Create(/*EmuWindow* render_window*/)
{
    s_render_window->MakeCurrent();
	glewInit(); // library

	if (!GLEW_ARB_vertex_shader || !GLEW_ARB_fragment_shader)
	{
		printf("OPENGL Warning hardware vertex and/or fragment shaders not supported!");
	}

	if (!GLEW_ARB_texture_non_power_of_two)
	{
		printf("OPENGL Warning hardware non-power-of-two textures not supported!");
	}

	gl.opened = true;
	OPENGL_Initialize();
}


////////////////////////////////////////////////////////////////////////////////
// EOF
