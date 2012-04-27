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
#include <glm/gtc/matrix_transform.hpp> 

GLuint g_fb_quad_buffer;

static const f32 g_identity[16] = {
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f
};

static int verts_per_frame = 0;
//static int g_offset_in_bytes = 0;

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

/// RendererGL3 constructor
RendererGL3::RendererGL3() {
    fbo_ = 0;          
    fbo_depth_;    
    fbo_texture_;  
    resolution_width_ = 640;
    resolution_height_ = 480;
    vbo_handle_ = 0;
    vbo_ = NULL;
    vbo_write_ofs_ = 0;
    vertex_position_format_ = 0;
    vertex_position_format_size_ = 0;
    vertex_position_component_count_ = (GXCompCnt)0;
    vertex_num_ = 0;                       
    render_window_ = NULL;
    generic_shader_id_ = 0;
}

/// Sets up the renderer for drawing a primitive
void RendererGL3::BeginPrimitive(GXPrimitive prim, int count) {
    static u32 flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT; //GL_MAP_FLUSH_EXPLICIT_BIT;
    int size = count * sizeof(GXVertex);
    vertex_num_ = 0;

    // Bind pointers to buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    vbo_ = (GXVertex*)glMapBufferRange(GL_ARRAY_BUFFER, vbo_write_ofs_, size, flags);
    if (vbo_ == NULL) {
        LOG_ERROR(TVIDEO, "Unable to map vertex buffer object to system mem!");
    }

    vbo_write_ofs_ += size;
}

/**
 * Set the type of postion vertex data
 * @param type Position data type (e.g. GX_F32)
 * @param count Position data count (e.g. GX_POS_XYZ)
 */
void RendererGL3::VertexPosition_SetType(GXCompType type, GXCompCnt count) {
    static GLuint gl_types[5] = {GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_FLOAT};
    static GLuint gl_types_size[5] = {1, 1, 2, 2, 4};
    vertex_position_format_ = gl_types[type];
    vertex_position_format_size_ = gl_types_size[type];
    vertex_position_component_count_ = count;
}

/**
 * Send a position vector to the renderer as 32-bit floating point
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
void RendererGL3::VertexPosition_SendFloat(f32* vec) {
    f32* ptr = (f32*)vbo_->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a position vector to the renderer as 16-bit short (signed or unsigned)
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
void RendererGL3::VertexPosition_SendShort(u16* vec) {
    u16* ptr = (u16*)vbo_->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a position vector to the renderer an 8-bit byte (signed or unsigned)
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
void RendererGL3::VertexPosition_SendByte(u8* vec) {
    u8* ptr = (u8*)vbo_->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Set the type of color 0 vertex data - type is always RGB8/RGBA8, just set count
 * @param count Color data count (e.g. GX_CLR_RGBA)
 */
void RendererGL3::VertexColor0_SetType(GXCompCnt count) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a vertex color 0 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
 * @param color Color to send, packed as RRGGBBAA or RRGGBB00
 */
void RendererGL3::VertexColor0_Send(u32 color) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Set the type of color 1 vertex data - type is always RGB8/RGBA8, just set count
 * @param count Color data count (e.g. GX_CLR_RGBA)
 */
void RendererGL3::VertexColor1_SetType(GXCompCnt count) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a vertex color 1 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
 * @param color Color to send, packed as RRGGBBAA or RRGGBB00
 */
void RendererGL3::VertexColor1_Send(u32 color) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Set the type of texture coordinate vertex data
 * @param texcoord 0-7 texcoord to set type of
 * @param type Texcoord data type (e.g. GX_F32)
 * @param count Texcoord data count (e.g. GX_TEX_ST)
 */
void RendererGL3::VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a texcoord vector to the renderer as 32-bit floating point
 * @param texcoord 0-7 texcoord to configure
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendFloat(int texcoord, f32* vec) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a texcoord vector to the renderer as 16-bit short (signed or unsigned)
 * @param texcoord 0-7 texcoord to configure
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendShort(int texcoord, u16* vec) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a texcoord vector to the renderer as 8-bit byte (signed or unsigned)
 * @param texcoord 0-7 texcoord to configure
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendByte(int texcoord, u8* vec) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/// Done with the current vertex - go to the next
void RendererGL3::VertexNext() {
    vbo_++;
    vertex_num_++;
}

/// Draws a primitive from the previously decoded vertex array
void RendererGL3::EndPrimitive() {
    //glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    //int position_buffer_size = (gp::g_position_burst_ptr - gp::g_position_burst_buffer);
    //int color0_buffer_size = (gp::g_color_burst_ptr - gp::g_color_burst_buffer) * 4;

    //glUnmapBuffer(GL_ARRAY_BUFFER);

    f32* pmtx = XF_GEOMETRY_MATRIX;
    f32 pmtx44[16];

    int i = 0, j = 0, k = 15;

    // convert 4x3 ode to gl 4x4
    pmtx44[0]  = pmtx[0]; pmtx44[1]  = pmtx[4]; pmtx44[2]  = pmtx[8]; pmtx44[3]  = 0;
    pmtx44[4]  = pmtx[1]; pmtx44[5]  = pmtx[5]; pmtx44[6]  = pmtx[9]; pmtx44[7]  = 0;
    pmtx44[8]  = pmtx[2]; pmtx44[9]  = pmtx[6]; pmtx44[10] = pmtx[10];pmtx44[11] = 0;
    pmtx44[12] = pmtx[3]; pmtx44[13] = pmtx[7]; pmtx44[14] = pmtx[11]; pmtx44[15] = 1;

    // Update XF matrices
    GLuint m_id = glGetUniformLocation(generic_shader_id_, "projectionMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &gp::g_projection_matrix[0]);

    m_id = glGetUniformLocation(generic_shader_id_, "modelMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &pmtx44[0]);
    
    /*
    s16 quad_buff[0x1000];
    int new_col_size = 0;
    int new_color_size = 0;
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
        
        //quad_col_buff[new_size+0] = gp::g_color_burst_buffer[i+0]; // 0

        new_size+=18;
        i+=12;
    }*/
    
    //new_size = position_buffer_size + position_buffer_size/2;

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    //glBufferSubData(GL_ARRAY_BUFFER, g_offset_in_bytes*4, new_size*4, quad_buff);

    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_SHORT,           // type
        GL_FALSE,           // normalized?
        sizeof(GXVertex),   // stride
        BUFFER_OFFSET(0)            // array buffer offset
        );
    
    // Draw the triangle !
    glDrawArrays(GL_LINES_ADJACENCY, 0, vertex_num_); // Starting from vertex 0; 3 vertices total -> 1 triangle
    //g_offset_in_bytes += new_size;

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
        */
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

/// Swap buffers (render frame)
void RendererGL3::SwapBuffers() {
    //RenderFramebuffer();
    render_window_->SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static u32 swaps = 0, last = 0;
	u32 t = SDL_GetTicks();
	swaps++;
		
	if(t - last > 500) {
        char title[100];
		f32 fps = 1000.0f * swaps / (t - last);
		swaps = 0;
		last = t;
		sprintf(title, "gekko-glfw - %02.02f fps", fps);
        render_window_->SetTitle(title);
	}
    vbo_write_ofs_ = 0; // Reset VBO position
}

/// Set the window of the emulator
void RendererGL3::SetWindow(EmuWindow* window) {
    render_window_ = window;
}

/// Shutdown the renderer
void RendererGL3::ShutDown() {
    printf("RendererGL3::Init()\n");
}

/// Renders the framebuffer quad to the screen
void RendererGL3::RenderFramebuffer() {
/*    // Framebuffer quad: XXXYY : X is position, Y is texcoordd
    static f32 fb_quad_verts[30] = {
        -1.0f,  -1.0f,  0.0f,   0.0f,   0.0f, 
        -1.0f,  1.0f,   0.0f,   0.0f,   1.0f,
        1.0f,   1.0f,   0.0f,   1.0f,   1.0f,
        1.0f,   1.0f,   0.0f,   1.0f,   1.0f,
        1.0f,   -1.0f,  0.0f,   1.0f,   0.0f,
        -1.0f,  -1.0f,  0.0f,   0.0f,   0.0f
    };

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
    // Don't transform - Load identity matrix
    glUniformMatrix4fv(glGetUniformLocation(generic_shader_id_, "projectionMatrix"), 1, GL_FALSE, g_identity);
    glUniformMatrix4fv(glGetUniformLocation(generic_shader_id_, "modelMatrix"), 1, GL_FALSE, g_identity);

    // Load vertex data
    glBindBuffer(GL_ARRAY_BUFFER, g_fb_quad_buffer);
    glBufferData(GL_ARRAY_BUFFER, 30*4, fb_quad_verts, GL_STATIC_DRAW);

    // Draw framebuffer quad
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_fb_quad_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /*
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_fb_quad_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, BUFFER_OFFSET(12));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    */

    //glDisableVertexAttribArray(0);


}

void RendererGL3::InitFramebuffer() {
    // Init the depth buffer
  /*  glGenRenderbuffers(1, &fbo_depth); // Generate one render buffer and store the ID in fbo_depth  
    glBindRenderbuffer(GL_RENDERBUFFER, fbo_depth); // Bind the fbo_depth render buffer 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution_width_, 
        resolution_height_); // Set the render buffer storage to be a depth component
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 
        fbo_depth); // Set the render buffer of this buffer to the depth buffer 
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // Unbind the render buffer  

    // Init the framebuffer texture
    glGenTextures(1, &fbo_texture); // Generate one texture  
    glBindTexture(GL_TEXTURE_2D, fbo_texture); // Bind the texture fbo_texture  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution_width_, resolution_height_, 0, GL_RGBA, 
        GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture  

    // Init the framebuffer display quad
    glGenBuffers(1, &g_fb_quad_buffer);
    */
} 

/// Generate vertex and fragment shader programs
GLuint GenerateShader(const char * vs, const char* gs, const char* fs){
    // Create the shaders
    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint gs_id = glCreateShader(GL_GEOMETRY_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);
 
    // Compile Vertex Shader
    glShaderSource(vs_id, 1, &vs , NULL);
    glCompileShader(vs_id);
 
    // Compile Geometry Shader
    glShaderSource(gs_id, 1, &gs , NULL);
    glCompileShader(gs_id);
 
    // Compile Fragment Shader
    glShaderSource(fs_id, 1, &fs , NULL);
    glCompileShader(fs_id);
 
    // Link the program
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vs_id);
    glAttachShader(program_id, gs_id);
    glAttachShader(program_id, fs_id);
    glLinkProgram(program_id);
 
    // Cleanup
    glDeleteShader(vs_id);
    glDeleteShader(gs_id);
    glDeleteShader(fs_id);
 
    return program_id;
}

/// Initialize the renderer and create a window
void RendererGL3::Init() {


    //glfwSwapInterval( 1 );

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture  
    glEnable(GL_DEPTH_TEST); // Enable depth testing
    glPolygonMode(GL_FRONT, GL_FILL);
    glFrontFace(GL_CW);
    glShadeModel(GL_SMOOTH);

    glfwEnable( GLFW_STICKY_KEYS );

    // GL extensions
    // -------------

    GLenum err = glewInit();
	if (GLEW_OK != err) {
        LOG_ERROR(TVIDEO, " Failed to initialize GLEW! Exiting...");
        glfwTerminate();
        exit(E_ERR);
	}

    // Initialize vertex buffers
    // -------------------------

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vbo_handle_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 1024*1024*16, NULL, GL_DYNAMIC_DRAW); // 16MB VBO - bigger? :D
                                                        // NULL - allocate, but not initialize

    // Initialize the framebuffer
    // --------------------------
/*
    InitFramebuffer();

    glGenFramebuffers(1, &fbo); // Generate one frame buffer and store the ID in fbo  
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); // Bind our frame buffer  

    // Attach the texture fbo_texture to the color buffer in our frame buffer 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

    // Attach the depth buffer fbo_depth to our frame buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depth);  

    // Check that the FBO initialized OK
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
    if (status != GL_FRAMEBUFFER_COMPLETE) {  
        LOG_ERROR(TVIDEO, "Couldn't create frame buffer");
        exit(1);
    } 
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind our frame buffer 
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

    char gs[1024] = "#version 150\n" \
                    "precision highp float;\n" \
                    "layout (lines_adjacency) in;\n" \
                    "layout (triangle_strip) out;\n" \
                    "layout (max_vertices = 4) out;\n" \
                    "void main(void) {\n" \
                    "   int i;\n" \
                    "   gl_Position = gl_in[0].gl_Position;\n" \
                    "   EmitVertex();\n" \
                    "   gl_Position = gl_in[1].gl_Position;\n" \
                    "   EmitVertex();\n" \
                    "   gl_Position = gl_in[3].gl_Position;\n" \
                    "   EmitVertex();\n" \
                    "   gl_Position = gl_in[2].gl_Position;\n" \
                    "   EmitVertex();\n" \
                    "   EndPrimitive();\n" \
                    "}";
 
    char fs[1024] = "#version 150\n" \
                    "in vec3 fragmentColor;\n" \
                    "out vec3 color;\n" \
                    "void main() {\n" \
                    "    color = vec3(1.0f, 1.0f, 1.0f);\n" \
                    "}\n";

    generic_shader_id_ = GenerateShader(vs, gs, fs);
    glUseProgram(generic_shader_id_);

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
    
    
    GLuint proj_id = glGetUniformLocation(generic_shader_id_, "projectionMatrix");
    glUniformMatrix4fv(proj_id, 1, GL_FALSE, &identity[0][0]);



   // GLuint view_id = glGetUniformLocation(generic_shader_id_, "viewMatrix");
   // glUniformMatrix4fv(view_id, 1, GL_FALSE, &Model[0][0]);
    
    GLuint model_id = glGetUniformLocation(generic_shader_id_, "modelMatrix");
    glUniformMatrix4fv(model_id, 1, GL_FALSE, &identity[0][0]);
}