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

/// Default Vertex Shader
const char kDefaultVertexShader[] = 
    "#version 150\n" \
    "layout(location = 0) in vec3 position;\n" \
    "layout(location = 1) in vec4 col0;\n" \
    "uniform mat4 projectionMatrix;\n" \
    "uniform mat4 modelMatrix;\n" \
    "out vec4 vertexColor;\n" \
    "void main() {\n" \
    "    gl_Position = projectionMatrix * modelMatrix * vec4(position, 1.0);\n" \
    "    vertexColor = vec4(col0[3]/255.0f, col0[2]/255.0f, col0[1]/255.0f, col0[0]/255.0f);\n" \
    "}";

/// Default Geometry Shader (Quads), otherwise disabled
const char kDefaultGeometryShaderQuads[] =
    "#version 150\n" \
    "precision highp float;\n" \
    "layout (lines_adjacency) in;\n" \
    "layout (triangle_strip) out;\n" \
    "layout (max_vertices = 4) out;\n" \
    "in vec4 vertexColor[];\n" \
    "out vec4 gsColor;\n" \
    "void main(void) {\n" \
    "   gl_Position = gl_in[0].gl_Position;\n" \
    "   gsColor = vertexColor[0];\n" \
    "   EmitVertex();\n" \
    "   gl_Position = gl_in[1].gl_Position;\n" \
    "   gsColor = vertexColor[1];\n" \
    "   EmitVertex();\n" \
    "   gl_Position = gl_in[3].gl_Position;\n" \
    "   gsColor = vertexColor[2];\n" \
    "   EmitVertex();\n" \
    "   gl_Position = gl_in[2].gl_Position;\n" \
    "   gsColor = vertexColor[3];\n" \
    "   EmitVertex();\n" \
    "   EndPrimitive();\n" \
    "}";

/// Default fragment shader
const char kDefaultFragmentShader[] =
    "#version 150\n" \
    "in vec4 vertexColor;\n" \
    "out vec3 fragmentColor;\n" \
    "void main() {\n" \
    "    fragmentColor = vertexColor.rgb;\n" \
    "}\n";

/// Default fragment shader for Quads
const char kDefaultFragmentShaderQuads[] =
    "#version 150\n" \
    "in vec4 gsColor;\n" \
    "out vec3 fragmentColor;\n" \
    "void main() {\n" \
    "    fragmentColor = gsColor.rgb;\n" \
    "}\n";

#endif // VIDEO_CORE_SHADER_BASE_TYPES_H_