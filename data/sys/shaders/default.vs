#version 150
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_separate_shader_objects : enable

uniform mat4 projection_matrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec4 color1;
layout(location = 3) in vec4 normal;

layout(location = 4) in vec3 texcoord0;

layout(location = 8) in vec4 m_idx_a;
layout(location = 9) in vec4 m_idx_b;
layout(location = 10) in vec4 m_idx_c;

// XF memory
uniform vec4 xf_modelview_vectors[3];
uniform vec4 xf_position_vectors[64];

// CP memory
uniform int cp_pos_shift;
uniform int cp_tex_shift_0;

// Vertex shader outputs
out vec4 vertexColor;
out vec2 vertexTexCoord0;

mat4 convert_matrix(in vec4 v0, in vec4 v1, in vec4 v2) {
    return mat4(v0[0], v1[0], v2[0], 0.0,
                v0[1], v1[1], v2[1], 0.0,
                v0[2], v1[2], v2[2], 0.0,
                v0[3], v1[3], v2[3], 1.0);
}

void main() {
    mat4 modelview_matrix;
    float cp_pos_dqf = 1.0 / float(1 << cp_pos_shift);
    float cp_tex_dqf_0 = 1.0 / float(1 << cp_tex_shift_0);
    
    if (m_idx_a[0] != 0) {
        modelview_matrix = convert_matrix(xf_position_vectors[uint(m_idx_a[0])],
                                          xf_position_vectors[uint(m_idx_a[0]) + 1],
                                          xf_position_vectors[uint(m_idx_a[0]) + 2]);
    } else {
        modelview_matrix = convert_matrix(xf_modelview_vectors[0],
                                          xf_modelview_vectors[1],
                                          xf_modelview_vectors[2]);
    }
    gl_Position = projection_matrix * modelview_matrix * vec4(position.xyz * cp_pos_dqf, 1.0);
    vertexTexCoord0 = texcoord0.st * cp_tex_dqf_0;
    vertexColor = clamp((color0.abgr / 255.0f), 0.0, 1.0);
}