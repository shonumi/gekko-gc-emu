// opengl.cpp
// (c) 2005,2008 Gekko Team / Wiimu Project

#include "emu.h"
#include "gx\gx_fifo.h"
#include "low level\hardware core\hw_vi.h"
#include "gx\gx_tev.h"
#include "gx\gx_texture.h"

opengl gl;

////////////////////////////////////////////////////////////////////////////////

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
    if(cfg.enb_framebuffer)	
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
    }
}

void OPENGL_Kill()
{
	if (gl.fullscreen)										
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

	if(gl.hdc && !ReleaseDC(wnd.hWnd, gl.hdc))				
	{
		printf("OPENGL Release Device Context Failed.");
		gl.hdc = NULL;										
	}

	gl.opened = false;
}

void OPENGL_Render()
{
	glFlush();
	SwapBuffers(gl.hdc);
}

void OPENGL_Create(HWND _hwnd)
{
	PIXELFORMATDESCRIPTOR pfd;
	DEVMODE dmScreenSettings; // Device Mode
	RECT rect;
	int format;

	OPENGL_Kill();

	if(gl.fullscreen && emu.started == true)
	{
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings)); // Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings); // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = gl.video_width; // Selected Screen Width
		dmScreenSettings.dmPelsHeight = gl.video_height; // Selected Screen Height
		dmScreenSettings.dmBitsPerPel = 32;	// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN)== DISP_CHANGE_SUCCESSFUL)
		{
			ShowCursor(FALSE); // Hide Mouse Pointer
			SetWindowLong(_hwnd, GWL_STYLE, WS_BORDER | WS_VISIBLE);
			SetWindowLong(_hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);
			SetMenu(_hwnd, NULL);
		}

		MoveWindow(_hwnd, 0, 0, gl.video_width, gl.video_height, true);
	}else{
		GetWindowRect(wnd.hWnd, &rect);

		rect.bottom = rect.top + gl.video_height;
		rect.right = rect.left + gl.video_width;
		int x = rect.left;
		int y = rect.top;

		AdjustWindowRectEx(&rect, WS_CAPTION | WS_VISIBLE | WS_BORDER, TRUE, NULL);
	}
	
	gl.hdc = GetDC(_hwnd);
	
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(gl.hdc, &pfd);
	SetPixelFormat(gl.hdc, format, &pfd);

	gl.hrc = wglCreateContext(gl.hdc);
	wglMakeCurrent(gl.hdc, gl.hrc);

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
