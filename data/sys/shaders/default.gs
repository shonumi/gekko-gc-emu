#version 150

precision highp float;
layout (lines_adjacency) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in vec4 vertexColor[];
in vec2 vertexTexCoord0[];
out vec4 gsColor;
out vec2 gsTexCoord0;

void main(void) {
    gl_Position = gl_in[0].gl_Position;
    gsColor = vertexColor[0];
    gsTexCoord0 = vertexTexCoord0[0].st;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gsColor = vertexColor[1];
    gsTexCoord0 = vertexTexCoord0[1].st;
    EmitVertex();
    
    gl_Position = gl_in[3].gl_Position;
    gsColor = vertexColor[3];
    gsTexCoord0 = vertexTexCoord0[3].st;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    gsColor = vertexColor[2];
    gsTexCoord0 = vertexTexCoord0[2].st;
    EmitVertex();

    EndPrimitive();
}