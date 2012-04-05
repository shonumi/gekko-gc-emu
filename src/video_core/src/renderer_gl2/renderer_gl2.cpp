/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    renderer_gl2.cpp
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-18
* \brief   Implementation of a OpenGL 2 renderer
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

#include "renderer_gl2.h"

#include <glm/glm.hpp>  
//#include <glm/gtc/matrix_projection.hpp>  
#include <glm/gtc/matrix_transform.hpp> 

GLuint g_position_buffer;
GLuint g_color0_buffer;
GLuint g_color1_buffer;
#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

static int offset_in_bytes = 0;

/// Draw a vertex array
void RendererGL2::DrawPrimitive() {
    
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

    // Update XF matrices
    GLuint m_id = glGetUniformLocation(shader_id_, "projectionMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &gp::g_projection_matrix[0]);
    //m_id = glGetUniformLocation(shader_id_, "viewMatrix");
    //glUniformMatrix4fv(m_id, 1, GL_FALSE, &identity[0][0]);


    m_id = glGetUniformLocation(shader_id_, "modelMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &pmtx44[0]);
    
    f32 quad_buff[0x1000];
    memset(quad_buff, 0, 0x1000);

    f32 quad_col_buff[0x1000];
    memset(quad_col_buff, 0, 0x1000);
    int new_col_size = 0;
    int new_size = 0;
    int col_i = 0;
    for (int i = 0; i < position_buffer_size;) {
        quad_buff[new_size+0] = gp::g_position_burst_buffer[i+0];
        quad_buff[new_size+1] = gp::g_position_burst_buffer[i+1];
        quad_buff[new_size+2] = gp::g_position_burst_buffer[i+2];

        quad_buff[new_size+3] = gp::g_position_burst_buffer[i+3];
        quad_buff[new_size+4] = gp::g_position_burst_buffer[i+4];
        quad_buff[new_size+5] = gp::g_position_burst_buffer[i+5];

        quad_buff[new_size+6] = gp::g_position_burst_buffer[i+6];
        quad_buff[new_size+7] = gp::g_position_burst_buffer[i+7];
        quad_buff[new_size+8] = gp::g_position_burst_buffer[i+8];

        quad_buff[new_size+9] = gp::g_position_burst_buffer[i+6];
        quad_buff[new_size+10] = gp::g_position_burst_buffer[i+7];
        quad_buff[new_size+11] = gp::g_position_burst_buffer[i+8];

        quad_buff[new_size+12] = gp::g_position_burst_buffer[i+9];
        quad_buff[new_size+13] = gp::g_position_burst_buffer[i+10];
        quad_buff[new_size+14] = gp::g_position_burst_buffer[i+11];

        quad_buff[new_size+15] = gp::g_position_burst_buffer[i+0];
        quad_buff[new_size+16] = gp::g_position_burst_buffer[i+1];
        quad_buff[new_size+17] = gp::g_position_burst_buffer[i+2];
        /*
        quad_col_buff[new_size+0] = gp::g_color_burst_buffer[i+0];
        quad_col_buff[new_size+1] = gp::g_color_burst_buffer[i+1];
        quad_col_buff[new_size+2] = gp::g_color_burst_buffer[i+2];

        quad_col_buff[new_size+3] = gp::g_color_burst_buffer[i+3];
        quad_col_buff[new_size+4] = gp::g_color_burst_buffer[i+4];
        quad_col_buff[new_size+5] = gp::g_color_burst_buffer[i+5];

        quad_col_buff[new_size+6] = gp::g_color_burst_buffer[i+6];
        quad_col_buff[new_size+7] = gp::g_color_burst_buffer[i+7];
        quad_col_buff[new_size+8] = gp::g_color_burst_buffer[i+8];

        quad_col_buff[new_size+9] = gp::g_color_burst_buffer[i+6];
        quad_col_buff[new_size+10] = gp::g_color_burst_buffer[i+7];
        quad_col_buff[new_size+11] = gp::g_color_burst_buffer[i+8];

        quad_col_buff[new_size+12] = gp::g_color_burst_buffer[i+9];
        quad_col_buff[new_size+13] = gp::g_color_burst_buffer[i+10];
        quad_col_buff[new_size+14] = gp::g_color_burst_buffer[i+11];

        quad_col_buff[new_size+15] = gp::g_color_burst_buffer[i+0];
        quad_col_buff[new_size+16] = gp::g_color_burst_buffer[i+1];
        quad_col_buff[new_size+17] = gp::g_color_burst_buffer[i+2];*/


        new_size+=18;
        i+=12;
    }
    
    new_size = position_buffer_size + position_buffer_size/2;

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, g_position_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, offset_in_bytes*4, new_size*4, quad_buff);
    //offset_in_bytes += new_size;
    
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        BUFFER_OFFSET(offset_in_bytes*4)            // array buffer offset
        );
    
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, new_size/3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    //glDrawElements(GL_TRIANGLES, new_size, GL_FLOAT, 0);

    offset_in_bytes += new_size;
    /**///glDrawElements(GL_TRIANGLES, position_buffer_size, GL_FLOAT, gp::g_position_burst_buffer);
    glDisableVertexAttribArray(0);
    
    ///////////////////////////////////////////////////////////////////////
  /*  
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, g_color0_buffer);
    glBufferData(GL_ARRAY_BUFFER, new_size*4, quad_col_buff, GL_STATIC_DRAW);

    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                  // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
        );
    //glDisableVertexAttribArray(1);*/
    
    
    //printf("RendererGL2::DrawPrimitive()\n");
}


/// Sets the renderer viewport location, width, and height
void RendererGL2::SetViewport(int x, int y, int width, int height) {
    glViewport(x, ((480 - (y + height))), width, height);
}

/// Sets the renderer depthrange, znear and zfar
void RendererGL2::SetDepthRange(double znear, double zfar) {
    glDepthRange(znear, zfar);
}

/// Sets the renderer depth test mode
void RendererGL2::SetDepthTest() {
}

/// Sets the renderer culling mode
void RendererGL2::SetCullMode() {
}

/// Sets the projection matrix
/*void RendererGL2::SetProjection(f32* mtx) {
glMatrixMode(GL_PROJECTION);
glLoadMatrixf((GLfloat*)mtx);
glMatrixMode(GL_MODELVIEW);
}*/

/// Swap buffers (render frame)
void RendererGL2::SwapBuffers() {	
    glFlush();
    
    render_window_->SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    input_common::g_user_input->PollEvent();

    static u32 swaps = 0, last = 0;
    //static int fps = 0;

    u32 t = SDL_GetTicks();
    swaps++;

    if(t - last > 500) {
        char title[100];
        f32 fps = 1000.0f * swaps / (t - last);
        swaps = 0;
        last = t;
        sprintf(title, "gekko-gl2 - %02.02f fps", fps);
        render_window_->SetTitle(title);
    }

    offset_in_bytes = 0;
}

/// Set the window of the emulator
void RendererGL2::SetWindow(EmuWindow* window) {
    render_window_ = window;
}

/// Shutdown the renderer
void RendererGL2::ShutDown() {
    printf("RendererGL2::ShutDown()\n");
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
void RendererGL2::Init() {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //glfwEnable( GLFW_STICKY_KEYS );

    // GL extensions
    // -------------

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        LOG_ERROR(TVIDEO, " Failed to initialize GLEW! Exiting...");
        exit(E_ERR);
    }

    // This will identify our vertex buffer
    // GLuint vertexbuffer;

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &g_position_buffer);
    glGenBuffers(1, &g_color0_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, g_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, 1024*1024*4, NULL, GL_DYNAMIC_DRAW); // 4MB VBO :-) NULL - allocate, but not initialize



    //glGenBuffers(1, &g_color1_buffer);

    char vs[1024] = "#version 120\n" \
        "layout(location = 0) in vec3 position;\n" \
        "layout(location = 1) in vec3 vertexColor;\n" \
        "out vec3 fragmentColor;\n" \
        "uniform mat4 projectionMatrix;\n" \
        "uniform mat4 modelMatrix;\n" \
        "void main() {\n" \
        "    gl_Position = projectionMatrix * modelMatrix * vec4(position, 1.0);\n" \
        "    fragmentColor = vertexColor;\n" \
        "}";

    char fs[1024] = "#version 120\n" \
        "in vec3 fragmentColor;\n" \
        "out vec3 color;\n" \
        "void main() {\n" \
        "    color = vec3(1.0, 0.0, 0.0);\n" \
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


    glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
    glClearDepth(1.0f);									// Depth Buffer Setup
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFrontFace(GL_CW);

    // Wireframe
    if(0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			
    }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

}