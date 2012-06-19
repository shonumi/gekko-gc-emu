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
     "layout(location = 1) in vec3 vertexColor;\n" \
     "out vec3 fragmentColor;\n" \
     "uniform mat4 projectionMatrix;\n" \
     "uniform mat4 modelMatrix;\n" \
     "void main() {\n" \
     "    gl_Position = projectionMatrix * modelMatrix * vec4(position, 1.0);\n" \
     "    fragmentColor = vertexColor;\n" \
     "}";

/// Default Geometry Shader (Quads), otherwise disabled
const char kDefaultGeometryShaderQuads[] =
    "#version 150\n" \
    "precision highp float;\n" \
    "layout (lines_adjacency) in;\n" \
    "layout (triangle_strip) out;\n" \
    "layout (max_vertices = 4) out;\n" \
    "void main(void) {\n" \
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

/// Default fragment shader
const char kDefaultFragmentShader[] =
    "#version 150\n" \
    "in vec3 fragmentColor;\n" \
    "out vec3 color;\n" \
    "void main() {\n" \
    "    color = vec3(1.0f, 0.0f, 0.0f);\n" \
    "}\n";;

#endif // VIDEO_CORE_SHADER_BASE_TYPES_H_