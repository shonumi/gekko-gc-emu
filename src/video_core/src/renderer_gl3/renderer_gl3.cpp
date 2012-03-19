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
#include "cp_mem.h"
#include "xf_mem.h"

#include "renderer_gl3.h"

#include <glm/glm.hpp>  
//#include <glm/gtc/matrix_projection.hpp>  
#include <glm/gtc/matrix_transform.hpp> 

GLuint g_position_buffer;
GLuint g_color0_buffer;
GLuint g_color1_buffer;

/// Draw a vertex array
void RendererGL3::DrawPrimitive() {

    int position_buffer_size = (gp::g_position_burst_ptr - gp::g_position_burst_buffer);
    int color0_buffer_size = (gp::g_color_burst_ptr - gp::g_color_burst_buffer) * 4;


    //position_buffer_size *= 2;

    //f32* test =(f32*) malloc(position_buffer_size*4);


    
    f32* pmtx = XF_GEOMETRY_MATRIX;
    f32 pmtx44[16];

    //glm::mat4 Model      = glm::mat4(1.0f);

    int i = 0, j = 0, k = 15;

	// convert 4x3 ode to gl 4x4
	pmtx44[0]  = pmtx[0]; pmtx44[1]  = pmtx[4]; pmtx44[2]  = pmtx[8]; pmtx44[3]  = 0;
	pmtx44[4]  = pmtx[1]; pmtx44[5]  = pmtx[5]; pmtx44[6]  = pmtx[9]; pmtx44[7]  = 0;
	pmtx44[8]  = pmtx[2]; pmtx44[9]  = pmtx[6]; pmtx44[10] = pmtx[10];pmtx44[11] = 0;
	pmtx44[12] = pmtx[3]; pmtx44[13] = pmtx[7]; pmtx44[14] = pmtx[11]; pmtx44[15] = 1;


/*
    glm::mat4 Projection      = glm::mat4(1.0f);

    i = 0, j = 0, k = 0;

    for (i=3;i>=0;i--) {
        for (j=3;j>=0;j--) {
            Projection[j][i] = gp::g_projection_matrix[k];
            k++;
        }
    }*/

    // Update XF matrices
    GLuint m_id = glGetUniformLocation(shader_id_, "projectionMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &gp::g_projection_matrix[0]);
    //m_id = glGetUniformLocation(shader_id_, "viewMatrix");
    //glUniformMatrix4fv(m_id, 1, GL_FALSE, &identity[0][0]);


    m_id = glGetUniformLocation(shader_id_, "modelMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &pmtx44[0]);
/*
    f32 quad_buff[0x1000];

    int new_size = 0;
    for (int i = 0; i < position_buffer_size;) {
        quad_buff[new_size+0] = gp::g_position_burst_buffer[i+0];
        quad_buff[new_size+1] = gp::g_position_burst_buffer[i+1];
        quad_buff[new_size+2] = gp::g_position_burst_buffer[i+2];
        quad_buff[new_size+3] = gp::g_position_burst_buffer[i+1];
        quad_buff[new_size+4] = gp::g_position_burst_buffer[i+2];
        quad_buff[new_size+5] = gp::g_position_burst_buffer[i+3];
        new_size+=6;
        i+=4;
    }
    */



    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, g_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, position_buffer_size*4, gp::g_position_burst_buffer, GL_STATIC_DRAW);

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
    //glDrawElements(GL_TRIANGLES, position_buffer_size, GL_FLOAT, gp::g_position_burst_buffer);
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

/// Sets the renderer depth test mode
void RendererGL3::SetDepthTest() {
}

/// Sets the renderer culling mode
void RendererGL3::SetCullMode() {
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



	static u32 swaps = 0, last = 0;
	//static int fps = 0;

	u32 t = SDL_GetTicks();
	swaps++;
		
	if(t - last > 500) {
        char title[100];
		f32 fps = 1000.0f * swaps / (t - last);
		swaps = 0;
		last = t;
		sprintf(title, "gekko-glfw - %02.02f fps", fps);
        glfwSetWindowTitle(title);
	}

    /*
// setup code

    static u32 startclock = SDL_GetTicks();
    u32 deltaclock = 0;
    u32 currentFPS = 0;

// actual fps calculation inside loop

    deltaclock = SDL_GetTicks() - startclock;
    startclock = SDL_GetTicks();
		
    if (deltaclock != 0) {
        char title[100];
        currentFPS = 1000 / deltaclock;
        sprintf(title, "gekko-glfw - %d fps", currentFPS);
        glfwSetWindowTitle(title);
    }*/
  
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

/// Generate vertex and fragment shader programs
GLuint GenerateShader(const char * vertex_shader,const char * fragment_shader){
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
    // Compile Vertex Shader
    glShaderSource(VertexShaderID, 1, &vertex_shader , NULL);
    glCompileShader(VertexShaderID);
 
    // Compile Fragment Shader
    glShaderSource(FragmentShaderID, 1, &fragment_shader , NULL);
    glCompileShader(FragmentShaderID);
 
    // Link the program
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, VertexShaderID);
    glAttachShader(program_id, FragmentShaderID);
    glLinkProgram(program_id);
 
    // Cleanup
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
 
    return program_id;
}

/// Initialize the renderer and create a window
void RendererGL3::Init() {

    if(!glfwInit()) {
        LOG_ERROR(TVIDEO, "Failed to initialize GLFW! Exiting...");
        exit(E_ERR);
    }
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    //glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
    //glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   // glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 16); // 2X AA

    

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
    //glGenBuffers(1, &g_color1_buffer);
 
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

	char vs[1024] = "#version 150\n" \
                    "layout(location = 0) in vec3 position;\n" \
                    "layout(location = 1) in vec3 vertexColor;\n" \
                    "out vec3 fragmentColor;\n" \
                    "uniform mat4 projectionMatrix;\n" \
                    "uniform mat4 modelMatrix;\n" \
                    "void main() {\n" \
                    "    gl_Position = projectionMatrix * modelMatrix * vec4(position, 1.0);\n" \
                    "    fragmentColor = vertexColor;\n" \
                    "}";
 
    char fs[1024] = "#version 150\n" \
                    "in vec3 fragmentColor;\n" \
                    "out vec3 color;\n" \
                    "void main() {\n" \
                    "    color = fragmentColor;\n" \
                    "}\n";

    shader_id_ = GenerateShader(vs, fs);
    glUseProgram(shader_id_);

    glm::mat4 identity  = glm::mat4(1.0f);  // Changes for each model !
    glm::mat4 proj = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
// Camera matrix
glm::mat4 View       = glm::lookAt(
    glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
    glm::vec3(0,0,0), // and looks at the origin
    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
);
// Model matrix : an identity matrix (model will be at the origin)
glm::mat4 Model      = glm::mat4(1.0f);  // Changes for each model !
// Our ModelViewProjection : multiplication of our 3 matrices
glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    
    GLuint proj_id = glGetUniformLocation(shader_id_, "projectionMatrix");
    glUniformMatrix4fv(proj_id, 1, GL_FALSE, &identity[0][0]);



   // GLuint view_id = glGetUniformLocation(shader_id_, "viewMatrix");
   // glUniformMatrix4fv(view_id, 1, GL_FALSE, &Model[0][0]);
    
    GLuint model_id = glGetUniformLocation(shader_id_, "modelMatrix");
    glUniformMatrix4fv(model_id, 1, GL_FALSE, &identity[0][0]);
}