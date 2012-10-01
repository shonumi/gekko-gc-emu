#version 140
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_uniform_buffer_object : enable

#define GX_U8       0
#define GX_S8       1
#define GX_U16      2
#define GX_S16      3
#define GX_F32      4
    
#define GX_RGB565   0
#define GX_RGB8     1
#define GX_RGBX8    2
#define GX_RGBA4    3
#define GX_RGBA6    4
#define GX_RGBA8    5

uniform mat4 projection_matrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec4 color1;
layout(location = 3) in vec4 normal;

layout(location = 4) in vec3 texcoord0;

layout(location = 8) in vec4 m_idx_a;
layout(location = 9) in vec4 m_idx_b;
layout(location = 10) in vec4 m_idx_c;

// CP memory
uniform int cp_pos_format;
uniform int cp_col0_format;
uniform int cp_col1_format;
uniform int cp_pos_shift;
uniform int cp_tex_shift_0;
uniform int cp_pos_matrix_index;

// Vertex shader outputs
out vec4 vertexColor;
out vec2 vertexTexCoord0;

// XF memory

layout(std140) uniform XFRegisters {
	vec4 pos_mem[0x40];
} xf_regs;

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
        modelview_matrix = convert_matrix(xf_regs.pos_mem[int(m_idx_a[0])],
                                          xf_regs.pos_mem[int(m_idx_a[0]) + 1],
                                          xf_regs.pos_mem[int(m_idx_a[0]) + 2]);
    } else {
        modelview_matrix = convert_matrix(xf_regs.pos_mem[cp_pos_matrix_index],
                                          xf_regs.pos_mem[cp_pos_matrix_index + 1],
                                          xf_regs.pos_mem[cp_pos_matrix_index + 2]);
    }
    
    // Position shift (dequantization factor) only applicable to U8/S8/U16/S16 formats
    if (cp_pos_format != GX_F32) { 
        gl_Position = projection_matrix * modelview_matrix * vec4(position.xyz * cp_pos_dqf, 1.0);
    } else {
        gl_Position = projection_matrix * modelview_matrix * vec4(position.xyz, 1.0);
    }
    
    vertexTexCoord0 = texcoord0.st * cp_tex_dqf_0;
    
    // Vertex color 0 decoding (this should be 100% correct)
    switch (cp_col0_format) {
    case GX_RGB565:
        vertexColor.r = float(int(color0[1]) >> 3) / 31.0f;
        vertexColor.g = float(((int(color0[1]) & 0x7) << 3) | (int(color0[0]) >> 5)) / 63.0f;
        vertexColor.b = float(int(color0[0]) & 0x1F) / 31.0f;
        vertexColor.a = 1.0f;
        break;
        
    case GX_RGB8:
        vertexColor = vec4(clamp((color0.rgb / 255.0f), 0.0, 1.0), 1.0);
        break;
        
    case GX_RGBX8:
        vertexColor = vec4(clamp((color0.abg / 255.0f), 0.0, 1.0), 1.0);
        break;
        
    case GX_RGBA4:
        vertexColor.r = float(int(color0[1]) >> 4) / 15.0f;
        vertexColor.g = float(int(color0[1]) & 0xF) / 15.0f;
        vertexColor.b = float(int(color0[0]) >> 4) / 15.0f;
        vertexColor.a = float(int(color0[0]) & 0xF) / 15.0f;
        break;
        
    case GX_RGBA6:
        vertexColor.r = float(int(color0[0]) >> 2) / 63.0f;
        vertexColor.g = float(((int(color0[0]) & 0x3) << 4) | (int(color0[1]) >> 4)) / 63.0f;
        vertexColor.b = float(((int(color0[1]) & 0xF) << 2) | (int(color0[2]) >> 6)) / 63.0f;
        vertexColor.a = float(int(color0[2]) & 0x3F) / 63.0f;
        break;
        
    case GX_RGBA8:
        vertexColor = clamp((color0.abgr / 255.0f), 0.0, 1.0);
        break;
        
    default:
        vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
        break;
    }
}
