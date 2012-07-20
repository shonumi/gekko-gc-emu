/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_base_types.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-04-27
 * @brief   All predefined GLSL shaders
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

#ifndef VIDEO_CORE_SHADER_BASE_TYPES_H_
#define VIDEO_CORE_SHADER_BASE_TYPES_H_

// Default Vertex Shader
//      NOTE: Vertex coords (position, normal, and texture) are sent with the last (third or fourth)
//      coordinate as the shift value. It will always an 8-bit unsigned integer.
const char kDefaultVertexShader[] = 
    "#version 330\n"
    "layout(location = 0) in vec3 position;\n"
    "layout(location = 1) in vec4 color0;\n"
    "layout(location = 2) in vec4 color1;\n"
    "layout(location = 3) in vec4 normal;\n"
    "layout(location = 4) in vec2 texcoord0;\n"
    "layout(location = 5) in vec2 texcoord1;\n"
    "layout(location = 6) in vec2 texcoord2;\n"
    "layout(location = 7) in vec2 texcoord3;\n"
    "layout(location = 8) in vec2 texcoord4;\n"
    "layout(location = 9) in vec2 texcoord5;\n"
    "layout(location = 10) in vec2 texcoord6;\n"
    "layout(location = 11) in vec2 texcoord7;\n"
    "layout(location = 12) in vec4 m_idx_a;\n"
    "layout(location = 13) in vec4 m_idx_b;\n"
    "layout(location = 14) in vec4 m_idx_c;\n"
    "uniform mat4 projection_matrix;\n"
    "uniform vec4 modelview_vectors[3];\n"
    "uniform vec4 position_vectors[64];\n"
    "out vec4 vertexColor;\n"
    "out vec2 vertexTexCoord0;\n"
    "mat4 convert_matrix(in vec4 v0, in vec4 v1, in vec4 v2) {\n"
    "    return mat4(v0[0], v1[0], v2[0], 0.0,\n"
    "                v0[1], v1[1], v2[1], 0.0,\n"
    "                v0[2], v1[2], v2[2], 0.0,\n"
    "                v0[3], v1[3], v2[3], 1.0);\n"
    "}\n"
    "void main() {\n"
    "    mat4 modelview_matrix;\n"
    "    if (m_idx_a[0] != 0) {\n"
    "        modelview_matrix = convert_matrix(position_vectors[uint(m_idx_a[0])],\n"
    "                                          position_vectors[uint(m_idx_a[0]) + 1],\n"
    "                                          position_vectors[uint(m_idx_a[0]) + 2]);\n"
    "    } else {\n"
    "        modelview_matrix = convert_matrix(modelview_vectors[0],\n"
    "                                          modelview_vectors[1],\n"
    "                                          modelview_vectors[2]);\n"
    "    }\n"
    "    gl_Position = projection_matrix * modelview_matrix * vec4(position.xyz, 1.0);\n"
    "    vertexTexCoord0 = texcoord0.st;\n"
    "    vertexColor = vec4(color0[3]/255.0f, color0[2]/255.0f, color0[1]/255.0f,\n"
    "                                                           color0[0]/255.0f);\n"
    "}";



/// Default Geometry Shader (Quads), otherwise disabled
const char kDefaultGeometryShaderQuads[] =
    "#version 330\n"
    "precision highp float;\n"
    "layout (lines_adjacency) in;\n"
    "layout (triangle_strip) out;\n"
    "layout (max_vertices = 4) out;\n"
    "in vec4 vertexColor[];\n"
    "in vec2 vertexTexCoord0[];\n"
    "out vec4 gsColor;\n"
    "out vec2 gsTexCoord0;\n"
    "void main(void) {\n"
    "   gl_Position = gl_in[0].gl_Position;\n"
    "   gsColor = vertexColor[0];\n"
    "   gsTexCoord0 = vertexTexCoord0[0].st;\n"
    "   EmitVertex();\n"

    "   gl_Position = gl_in[1].gl_Position;\n"
    "   gsColor = vertexColor[1];\n"
    "   gsTexCoord0 = vertexTexCoord0[0].st;\n"
    "   EmitVertex();\n"

    "   gl_Position = gl_in[3].gl_Position;\n"
    "   gsColor = vertexColor[3];\n"
    "   gsTexCoord0 = vertexTexCoord0[3].st;\n"
    "   EmitVertex();\n"

    "   gl_Position = gl_in[2].gl_Position;\n"
    "   gsColor = vertexColor[2];\n"
    "   gsTexCoord0 = vertexTexCoord0[2].st;\n"
    "   EmitVertex();\n"

    "   EndPrimitive();\n"
    "}";

/// Default fragment shader
const char kDefaultFragmentShader[] =
    "#version 330\n"
    "uniform sampler2D texture0;\n"
    "in vec4 vertexColor;\n"
    "in vec2 vertexTexCoord0;\n"
    "out vec3 fragmentColor;\n"
    "void main() {\n"
    "    fragmentColor = texture(texture0, vertexTexCoord0.st).rgb;\n"
    //"    fragmentColor = vertexColor.rgb;\n"
    "}\n";

/// Default fragment shader for Quads
const char kDefaultFragmentShaderQuads[] =
    "#version 330\n"
    "uniform sampler2D texture0;\n"
    "in vec4 gsColor;\n"
    "in vec2 gsTexCoord0;\n"
    "out vec3 fragmentColor;\n"
    "void main() {\n"
    "    fragmentColor = texture(texture0, gsTexCoord0.st).rgb;\n"
    //"    fragmentColor = gsColor.rgb;\n"
    "}\n";

#endif // VIDEO_CORE_SHADER_BASE_TYPES_H_