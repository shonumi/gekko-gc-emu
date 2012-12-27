// Prepare and compute TEV stage result
#define STAGE_RESULT(s) \
    tex = _FSDEF_TEXTURE_##s; \
 \
    stage = fs_ubo.tev_stages[s]; \
    konst = stage.konst; \
 \
    stage_result = (vec4(_FSDEF_COMBINER_COLOR_D_##s, _FSDEF_COMBINER_ALPHA_D_##s) + \
        (vec4(stage.color_sub, stage.color_sub, stage.color_sub, stage.alpha_sub) * \
        (mix(vec4(_FSDEF_COMBINER_COLOR_A_##s, _FSDEF_COMBINER_ALPHA_A_##s), \
        vec4(_FSDEF_COMBINER_COLOR_B_##s, _FSDEF_COMBINER_ALPHA_B_##s), \
        vec4(_FSDEF_COMBINER_COLOR_C_##s, _FSDEF_COMBINER_ALPHA_C_##s)) + \
        vec4(stage.color_bias, stage.color_bias, stage.color_bias, stage.alpha_bias)))); \
 \
    _FSDEF_COMBINER_COLOR_DEST_##s = _FSDEF_CLAMP_COLOR_##s(stage.color_scale * \
        stage_result.rgb); \
    _FSDEF_COMBINER_ALPHA_DEST_##s = _FSDEF_CLAMP_ALPHA_##s(stage_result.a);
    
struct TevStage {
    float color_bias;
    float color_sub;
    float color_scale;

    float alpha_bias;
    float alpha_sub;
    float alpha_scale;
    
    int pad0;
    int pad1;
    
    vec4 konst;

    // NOTE: this struct must be padded to a 16 byte boundary in order to be tightly packed
};

struct TevState {
    int alpha_func_ref0;
    int alpha_func_ref1;

    int pad0;
    int pad1;
    
    vec4 color[4];

    // NOTE: this struct must be padded to a 16 byte boundary in order to be tightly packed
};

layout(std140) uniform _FS_UBO {
    TevState tev_state;
    TevStage tev_stages[16];
} fs_ubo;

// Textures
uniform sampler2D texture[8];
in vec4 vtx_color_0;
in vec2 vtx_texcoord[8];

out vec4 frag_dest;

void main() {
    float alpha; 
    TevStage stage;
    vec4 stage_result;

    vec4 prev = fs_ubo.tev_state.color[0];
    vec4 color0= fs_ubo.tev_state.color[1];
    vec4 color1 = fs_ubo.tev_state.color[2];
    vec4 color2 = fs_ubo.tev_state.color[3];
    vec4 tex;
    vec4 konst;
    vec4 ras = vtx_color_0;
    
    STAGE_RESULT(0);
#if _FSDEF_NUM_STAGES > 0
    STAGE_RESULT(1);
#endif
#if _FSDEF_NUM_STAGES > 1
    STAGE_RESULT(2);
#endif
#if _FSDEF_NUM_STAGES > 2
    STAGE_RESULT(3);
#endif
#if _FSDEF_NUM_STAGES > 3
    STAGE_RESULT(4);
#endif
#if _FSDEF_NUM_STAGES > 4
    STAGE_RESULT(5);
#endif
#if _FSDEF_NUM_STAGES > 5
    STAGE_RESULT(6);
#endif
#if _FSDEF_NUM_STAGES > 6
    STAGE_RESULT(7);
#endif
#if _FSDEF_NUM_STAGES > 7
    STAGE_RESULT(8);
#endif
#if _FSDEF_NUM_STAGES > 8
    STAGE_RESULT(9);
#endif
#if _FSDEF_NUM_STAGES > 9
    STAGE_RESULT(10);
#endif
#if _FSDEF_NUM_STAGES > 10
    STAGE_RESULT(11);
#endif
#if _FSDEF_NUM_STAGES > 11
    STAGE_RESULT(12);
#endif
#if _FSDEF_NUM_STAGES > 12
    STAGE_RESULT(13);
#endif
#if _FSDEF_NUM_STAGES > 13
    STAGE_RESULT(14);
#endif
#if _FSDEF_NUM_STAGES > 14
    STAGE_RESULT(15);
#endif
    frag_dest = _FSDEF_STAGE_DEST;
    
    // Alpha compare
    // -------------

    int val = int(frag_dest.a * 255.0f) & 0xFF;                                   
    if (_FSDEF_ALPHA_COMPARE(val, fs_ubo.tev_state.alpha_func_ref0, 
        fs_ubo.tev_state.alpha_func_ref1))
        discard;
}
