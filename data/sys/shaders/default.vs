layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec4 color1;
layout(location = 3) in vec4 normal;

layout(location = 4) in vec4 texcoord01;
layout(location = 5) in vec4 texcoord23;
layout(location = 6) in vec4 texcoord45;
layout(location = 7) in vec4 texcoord67;

layout(location = 8) in vec4 matrix_idx_pos;
layout(location = 9) in vec4 matrix_idx_tex03;
layout(location = 10) in vec4 matrix_idx_tex47;

struct VertexState {
    float cp_pos_dqf; 
    int cp_pos_matrix_offset;
    int pad0;
    int pad1;
    vec4 cp_tex_dqf[2];
    ivec4 cp_tex_matrix_offset[2];
    mat4 projection_matrix;
};

// XF memory
layout(std140) uniform _VS_UBO {
    VertexState state;
	vec4 xf_mem[0x40];
} vs_ubo;

// Vertex shader outputs
out vec4 vtx_color_0;
out vec2 vtx_texcoord[8];

#define XF_MEM_MTX44(addr) mat4( \
    vs_ubo.xf_mem[addr].x, vs_ubo.xf_mem[addr + 1].x, vs_ubo.xf_mem[addr + 2].x, 0.0, \
    vs_ubo.xf_mem[addr].y, vs_ubo.xf_mem[addr + 1].y, vs_ubo.xf_mem[addr + 2].y, 0.0, \
    vs_ubo.xf_mem[addr].z, vs_ubo.xf_mem[addr + 1].z, vs_ubo.xf_mem[addr + 2].z, 0.0, \
    vs_ubo.xf_mem[addr].w, vs_ubo.xf_mem[addr + 1].w, vs_ubo.xf_mem[addr + 2].w, 1.0)

void main() {
    mat4 modelview_matrix;
#ifdef __VSDEF_POS_MIDX // Position modelview matrix
    modelview_matrix = XF_MEM_MTX44(int(matrix_idx_pos[0]));
#else
    modelview_matrix = XF_MEM_MTX44(vs_ubo.state.cp_pos_matrix_offset);
#endif
#ifdef __VSDEF_POS_DQF // Position shift (dequantization factor) only U8/S8/U16/S16 formats
    gl_Position = vs_ubo.state.projection_matrix * modelview_matrix * 
        vec4(position.xyz * vs_ubo.state.cp_pos_dqf, 1.0);
#else
    gl_Position = vs_ubo.state.projection_matrix * modelview_matrix * vec4(position.xyz, 1.0);
#endif
#ifdef __VSDEF_TEX_0_MIDX // Texture coord 0
    vtx_texcoord[0] = vec4(XF_MEM_MTX44(int(matrix_idx_tex03[0])) * 
        vec4(texcoord01.xy * vs_ubo.state.cp_tex_dqf[0][0], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[0] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[0][0]) * 
        vec4(texcoord01.xy * vs_ubo.state.cp_tex_dqf[0][0], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_1_MIDX // Texture coord 1
    vtx_texcoord[1] = vec4(XF_MEM_MTX44(int(matrix_idx_tex03[1])) * 
        vec4(texcoord01.zw * vs_ubo.state.cp_tex_dqf[0][1], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[1] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[0][1]) * 
        vec4(texcoord01.zw * vs_ubo.state.cp_tex_dqf[0][1], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_2_MIDX // Texture coord 2
    vtx_texcoord[2] = vec4(XF_MEM_MTX44(int(matrix_idx_tex03[2])) * 
        vec4(texcoord23.xy * vs_ubo.state.cp_tex_dqf[0][2], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[2] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[0][2]) * 
        vec4(texcoord23.xy * vs_ubo.state.cp_tex_dqf[0][2], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_3_MIDX // Texture coord 3
    vtx_texcoord[3] = vec4(XF_MEM_MTX44(int(matrix_idx_tex03[3])) * 
        vec4(texcoord23.zw * vs_ubo.state.cp_tex_dqf[0][3], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[3] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[0][3]) * 
        vec4(texcoord23.zw * vs_ubo.state.cp_tex_dqf[0][3], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_4_MIDX // Texture coord 4
    vtx_texcoord[4] = vec4(XF_MEM_MTX44(int(matrix_idx_tex47[0])) * 
        vec4(texcoord45.xy * vs_ubo.state.cp_tex_dqf[1][0], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[4] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[1][0]) * 
        vec4(texcoord45.xy * vs_ubo.state.cp_tex_dqf[1][0], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_5_MIDX // Texture coord 5
    vtx_texcoord[5] = vec4(XF_MEM_MTX44(int(matrix_idx_tex47[1])) * 
        vec4(texcoord45.zw * vs_ubo.state.cp_tex_dqf[1][1], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[5] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[1][1]) * 
        vec4(texcoord45.zw * vs_ubo.state.cp_tex_dqf[1][1], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_6_MIDX // Texture coord 6
    vtx_texcoord[6] = vec4(XF_MEM_MTX44(int(matrix_idx_tex47[2])) * 
        vec4(texcoord67.xy * vs_ubo.state.cp_tex_dqf[1][2], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[6] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[1][2]) * 
        vec4(texcoord67.xy * vs_ubo.state.cp_tex_dqf[1][2], 0.0f, 1.0f)).st;
#endif
#ifdef __VSDEF_TEX_7_MIDX // Texture coord 7
    vtx_texcoord[7] = vec4(XF_MEM_MTX44(int(matrix_idx_tex47[3])) * 
        vec4(texcoord67.zw * vs_ubo.state.cp_tex_dqf[1][3], 0.0f, 1.0f)).st;
#else
    vtx_texcoord[7] = vec4(XF_MEM_MTX44(vs_ubo.state.cp_tex_matrix_offset[1][3]) * 
        vec4(texcoord67.zw * vs_ubo.state.cp_tex_dqf[1][3], 0.0f, 1.0f)).st;
#endif
    // Vertex color 0
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
