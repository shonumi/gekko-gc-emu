layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec4 color1;
layout(location = 3) in vec4 normal;

layout(location = 4) in vec4 texcoord01;
layout(location = 5) in vec4 texcoord23;
layout(location = 6) in vec4 texcoord45;
layout(location = 7) in vec4 texcoord67;

layout(location = 8) in vec4 m_idx_a;
layout(location = 9) in vec4 m_idx_b;
layout(location = 10) in vec4 m_idx_c;

// CP memory
uniform float cp_pos_dqf;
uniform float cp_tex_dqf[8];
uniform int cp_pos_matrix_index;

// XF memory
uniform mat4 projection_matrix;
layout(std140) uniform XFRegisters {
	vec4 pos_mem[0x40];
} xf_regs;

// Vertex shader outputs
out vec4 vtx_color_0;
out vec2 vtx_texcoord[8];

#define CONVERT_MATRIX(v0, v1, v2) mat4(v0[0], v1[0], v2[0], 0.0, v0[1], v1[1], v2[1], 0.0, v0[2], \
    v1[2], v2[2], 0.0, v0[3], v1[3], v2[3], 1.0);

void main() {
    mat4 modelview_matrix;
#ifdef __VSDEF_POS_MIDX
    modelview_matrix = CONVERT_MATRIX(xf_regs.pos_mem[int(m_idx_a[0])],
        xf_regs.pos_mem[int(m_idx_a[0]) + 1], xf_regs.pos_mem[int(m_idx_a[0]) + 2]);
#else
    modelview_matrix = CONVERT_MATRIX(xf_regs.pos_mem[cp_pos_matrix_index],
        xf_regs.pos_mem[cp_pos_matrix_index + 1], xf_regs.pos_mem[cp_pos_matrix_index + 2]);
#endif
    // Position shift (dequantization factor) only applicable to U8/S8/U16/S16 formats
#ifdef __VSDEF_POS_DQF
    gl_Position = projection_matrix * modelview_matrix * vec4(position.xyz * cp_pos_dqf, 1.0);
#else
    gl_Position = projection_matrix * modelview_matrix * vec4(position.xyz, 1.0);
#endif
    vtx_texcoord[0] = texcoord01.xy * cp_tex_dqf[0];
    vtx_texcoord[1] = texcoord01.zw * cp_tex_dqf[1];
    vtx_texcoord[2] = texcoord23.xy * cp_tex_dqf[2];
    vtx_texcoord[3] = texcoord23.zw * cp_tex_dqf[3];
    vtx_texcoord[4] = texcoord45.xy * cp_tex_dqf[4];
    vtx_texcoord[5] = texcoord45.zw * cp_tex_dqf[5];
    vtx_texcoord[6] = texcoord67.xy * cp_tex_dqf[6];
    vtx_texcoord[7] = texcoord67.zw * cp_tex_dqf[7];
    
    // Vertex color 0 decoding
#ifdef __VSDEF_COLOR0_RGB565
    vtx_color_0.r = float(int(color0[1]) >> 3) / 31.0f;
    vtx_color_0.g = float(((int(color0[1]) & 0x7) << 3) | (int(color0[0]) >> 5)) / 63.0f;
    vtx_color_0.b = float(int(color0[0]) & 0x1F) / 31.0f;
    vtx_color_0.a = 1.0f;
#elif defined(__VSDEF_COLOR0_RGB8)
    vtx_color_0 = vec4(clamp((color0.rgb / 255.0f), 0.0, 1.0), 1.0);
#elif defined(__VSDEF_COLOR0__RGBX8)
    vtx_color_0 = vec4(clamp((color0.abg / 255.0f), 0.0, 1.0), 1.0);
#elif defined(__VSDEF_COLOR0_RGBA4)
    vtx_color_0.r = float(int(color0[1]) >> 4) / 15.0f;
    vtx_color_0.g = float(int(color0[1]) & 0xF) / 15.0f;
    vtx_color_0.b = float(int(color0[0]) >> 4) / 15.0f;
    vtx_color_0.a = float(int(color0[0]) & 0xF) / 15.0f;
#elif defined(__VSDEF_COLOR0_RGBA6)
    vtx_color_0.r = float(int(color0[0]) >> 2) / 63.0f;
    vtx_color_0.g = float(((int(color0[0]) & 0x3) << 4) | (int(color0[1]) >> 4)) / 63.0f;
    vtx_color_0.b = float(((int(color0[1]) & 0xF) << 2) | (int(color0[2]) >> 6)) / 63.0f;
    vtx_color_0.a = float(int(color0[2]) & 0x3F) / 63.0f;
#elif defined(__VSDEF_COLOR0_RGBA8)
    vtx_color_0 = clamp((color0.abgr / 255.0f), 0.0, 1.0);
#else
    vtx_color_0 = vec4(1.0, 1.0, 1.0, 1.0);
#endif
}
