/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    renderer_gl3.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-10
 * \brief   Implementation of a OpenGL 3.2 renderer
 *
 * \section LICENSE
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */

#include "common.h"

#include "input_common.h"

#include "vertex_loader.h"

#include "renderer_gl3.h"

#include <GL/glew.h>
#include <GL/glfw.h>

GLuint g_position_buffer;
GLuint g_color0_buffer;
GLuint g_color1_buffer;

/// Draw a vertex array
void RendererGL3::DrawPrimitive() {

    int position_buffer_size = (gp::g_position_burst_ptr - gp::g_position_burst_buffer) * 4;
    int color0_buffer_size = (gp::g_color_burst_ptr - gp::g_color_burst_buffer) * 4;

    glEnableVertexAttribArray(0);



    glBindBuffer(GL_ARRAY_BUFFER, g_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, position_buffer_size, gp::g_position_burst_buffer, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
       0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );
 
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, position_buffer_size); // Starting from vertex 0; 3 vertices total -> 1 triangle
 
    glDisableVertexAttribArray(0);

    ///////////////////////////////////////////////////////////////////////

    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, g_color0_buffer);
    glBufferData(GL_ARRAY_BUFFER, color0_buffer_size, gp::g_color_burst_buffer, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                  // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    //glDisableVertexAttribArray(1);


    //printf("RendererGL3::DrawPrimitive()\n");
}


/// Sets the renderer viewport location, width, and height
void RendererGL3::SetViewport(int x, int y, int width, int height) {
    glViewport(x, ((480 - (y + height))), width, height);
}

/// Sets the renderer depthrange, znear and zfar
void RendererGL3::SetDepthRange(double znear, double zfar) {
    glDepthRange(znear, zfar);
}

/// Sets the projection matrix
/*void RendererGL3::SetProjection(f32* mtx) {
	glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((GLfloat*)mtx);
	glMatrixMode(GL_MODELVIEW);
}*/

/// Swap buffers (render frame)
void RendererGL3::SwapBuffers() {	
    glfwSwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT);

#if EMU_PLATFORM == PLATFORM_WINDOWS
	static u32 swaps = 0, last = 0;
	static float fps = 0;
	u32 t = GetTickCount ();
	swaps++;
		
	if(t - last > 1000) {
		fps = (float) swaps / (0.001 * (t - last));
		
		swaps = 0;
		last = t;

		char str1[100];

		sprintf(str1, "gekko-glfw - %03.02f fps", fps);
        glfwSetWindowTitle(str1);
	}
#endif
}

/*! 
 * \brief Set the window text of the renderer
 * \param text Text so set the window title bar to
 */
void RendererGL3::SetWindowText(const char* text) {
    printf("RendererGL3::SetWindowText()\n");
}

/*! 
 * \brief Set the window size of the renderer
 * \param text Text so set the window title bar to
 */
void RendererGL3::SetWindowSize(int width, int height) {
    printf("RendererGL3::SetWindowSize()\n");
}

/// Shutdown the renderer
void RendererGL3::ShutDown() {
    printf("RendererGL3::Init()\n");
}

void InitShaders(void)
{
	GLuint p, f, v;

	char vs[1024] = "#version 130\n" \
                    "layout(location = 0) in vec3 Position;\n" \
                    "void main() {\n" \
                    "    gl_Position.xyz = Position;\n" \
                    "}";
        
    char fs[1024] = "#version 130\n" \
                    "out vec3 color;\n" \
                    "void main() {\n" \
                    "    color = vec3(1,0,0);\n" \
                    "}\n";

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);	

	// load shaders & get length of each
	GLint vlen;
	GLint flen;
	//vs = loadFile("minimal.vert",vlen);
	//fs = loadFile("minimal.frag",flen);

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,&vlen);
	glShaderSource(f, 1, &ff,&flen);

	GLint compiled;

	glCompileShader(v);
	glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		LOG_ERROR(TGP, "Vertex shader not compiled.");
		//printShaderInfoLog(v);
	} 

	glCompileShader(f);
	glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		LOG_ERROR(TGP, "Fragment shader not compiled.");
		//printShaderInfoLog(f);
	} 

	p = glCreateProgram();

	glBindAttribLocation(p,0, "in_Position");
	glBindAttribLocation(p,1, "in_Color");

	glAttachShader(p,v);
	glAttachShader(p,f);

	glLinkProgram(p);
	glUseProgram(p);

//	delete [] vs; // dont forget to free allocated memory
//	delete [] fs; // we allocated this in the loadFile function...
}


GLuint GenerateShader(const char * vertex_shader,const char * fragment_shader){
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
    // Compile Vertex Shader
    glShaderSource(VertexShaderID, 1, &vertex_shader , NULL);
    glCompileShader(VertexShaderID);
 
    // Check Vertex Shader
    //glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    //glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
 
    // Compile Fragment Shader
    glShaderSource(FragmentShaderID, 1, &fragment_shader , NULL);
    glCompileShader(FragmentShaderID);
 
    // Check Fragment Shader
    //glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    //glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    // Link the program
    //fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
 
    // Check the program
    //glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    //glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
 
    return ProgramID;
}

/// Initialize the renderer and create a window
void RendererGL3::Init() {

    if(!glfwInit()) {
        LOG_ERROR(TVIDEO, "Failed to initialize GLFW! Exiting...");
        exit(E_ERR);
    }
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16); // 2X AA

    

    if(!glfwOpenWindow(resolution_width_, resolution_height_, 0, 0, 0, 0, 32, 0, GLFW_WINDOW)) {
        LOG_ERROR(TVIDEO, "Failed to open GLFW window! Exiting...");
        glfwTerminate();
        exit(E_ERR);
    }
    LOG_NOTICE(TVIDEO, "OpenGL Context: %d.%d initialzed ok\n", 
        glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR), 
        glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR));

    glfwSetWindowTitle("gekko-glfw");
    //glfwSwapInterval( 1 );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glfwEnable( GLFW_STICKY_KEYS );

    // GL extensions
    // -------------

    GLenum err = glewInit();
	if (GLEW_OK != err) {
        LOG_ERROR(TVIDEO, " Failed to initialize GLEW! Exiting...");
        glfwTerminate();
        exit(E_ERR);
	}











    // This will identify our vertex buffer
   // GLuint vertexbuffer;
 
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &g_position_buffer);
    glGenBuffers(1, &g_color0_buffer);
    glGenBuffers(1, &g_color1_buffer);
 
    // The following commands will talk about our 'vertexbuffer' buffer
  //  glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buffer);
 



    /*

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &g_vertex_buffer);
 
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buffer);
 
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(gp::g_position_burst_buffer), gp::g_position_burst_buffer, GL_DYNAMIC_DRAW);


    */

	char vs[1024] = "#version 130\n" \
                    "layout(location = 0) in vec3 Position;\n" \
                    "layout(location = 1) in vec3 vertexColor;\n" \
                    "out vec3 fragmentColor;\n" \
                    "void main() {\n" \
                    "    gl_Position.xyz = Position;\n" \
                    "    fragmentColor = vertexColor;\n" \
                    "}";
        
    char fs[1024] = "#version 130\n" \
                    "in vec3 fragmentColor;\n" \
                    "out vec3 color;\n" \
                    "void main() {\n" \
                    "    color = fragmentColor;\n" \
                    "}\n";

    GLuint programID = GenerateShader(vs, fs);
    glUseProgram(programID);
    //InitShaders();


    //do{
     //   glClear(GL_COLOR_BUFFER_BIT);

        // Draw nothing, see you in tutorial 2 !
         // 1rst attribute buffer : vertices
        /*glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
           0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
           3,                  // size
           GL_FLOAT,           // type
           GL_FALSE,           // normalized?
           0,                  // stride
           (void*)0            // array buffer offset
        );
 
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
 
        glDisableVertexAttribArray(0);*/

        

        // Swap buffers
      //  glfwSwapBuffers();
 
    //} // Check if the ESC key was pressed or the window was closed
    //while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
    //glfwGetWindowParam( GLFW_OPENED ) );
}