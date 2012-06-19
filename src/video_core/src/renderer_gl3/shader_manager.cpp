/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_manager.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-04-22
 * @brief   Managers shaders for the GL3 renderer
 *
 * @section LICENSE
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

#include "renderer_gl3.h"
#include "gx_types.h"
#include "shader_base_types.h"

namespace shader_manager {

GLuint shader_default_id;       ///< Handle to default shader program
GLuint shader_default_quads_id; ///< Handle to default quads shader program

/**
 * @brief Sets the primitive type for shader use
 * @param type GXPrimitive type of current primitive
 */
void SetPrimitive(GXPrimitive type) {
    //if (type == GX_QUADS) {
        glUseProgram(shader_default_quads_id);
    //} else {
    //    glUseProgram(shader_default_id);
   /// }
}

/// Initializes the default shaders for general use
GLuint CompileShaderProgram(const char * vs, const char* gs, const char* fs){
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

/// Initialize the shader manager
void Init() {

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

    //shader_default_id = CompileShaderProgram(kDefaultVertexShader, 
    //                                         NULL, 
    //                                         kDefaultFragmentShader);

    shader_default_quads_id = CompileShaderProgram(vs, 
                                                   gs,
                                                   fs);

    glUseProgram(shader_default_quads_id);

    LOG_NOTICE(TGP, "shader_manager initialized ok");
}

} // namespace