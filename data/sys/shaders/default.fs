// Adjust value to unsigned 8-bit
#define FIX_U8(val) (round((fract(val * 255.0f/256.0f) * 256.0f/255.0f) * 255.0f) / 255.0f)

#define FIX_U6(val) (round(val * 63.0f) / 63.0f)
#define FIX_U5(val) (round(val * 31.0f) / 31.0f)

// Prepare and compute TEV stage result
#define STAGE_RESULT(s) \
    tex = _FSDEF_TEXTURE_##s; \
    tex = _FSDEF_FIX_FORMAT_TEXTURE_##s; \
 \
    stage = tev_stages[s]; \
    konst = stage.konst; \
    ras = _FSDEF_RASCOLOR_##s; \
    scale = vec4(stage.color_scale, stage.color_scale, stage.color_scale, 1.0); \
    \
    reg_a = FIX_U8(vec4(_FSDEF_COMBINER_COLOR_A_##s, _FSDEF_COMBINER_ALPHA_A_##s)); \
    reg_b = FIX_U8(vec4(_FSDEF_COMBINER_COLOR_B_##s, _FSDEF_COMBINER_ALPHA_B_##s)); \
    reg_c = FIX_U8(vec4(_FSDEF_COMBINER_COLOR_C_##s, _FSDEF_COMBINER_ALPHA_C_##s)); \
    reg_d = vec4(_FSDEF_COMBINER_COLOR_D_##s, _FSDEF_COMBINER_ALPHA_D_##s); \
 \
    stage_result = scale * (reg_d + \
        (vec4(stage.color_sub, stage.color_sub, stage.color_sub, stage.alpha_sub) * \
        (mix(reg_a, reg_b, reg_c) + \
        vec4(stage.color_bias, stage.color_bias, stage.color_bias, stage.alpha_bias)))); \
 \
    _FSDEF_COMBINER_COLOR_DEST_##s = _FSDEF_CLAMP_COLOR_##s(stage_result.rgb); \
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

    int dest_alpha;
    int pad1;
    
    vec4 color[4];

    // NOTE: this struct must be padded to a 16 byte boundary in order to be tightly packed
};

layout(std140) uniform _FS_UBO {
    TevState tev_state;
    TevStage tev_stages[16];
};

// Textures
uniform sampler2D texture[8];
in vec4 vtx_color[2];
in vec2 vtx_texcoord[8];

out vec4 col0;
out vec4 col1;

void main() {
    float alpha; 
    TevStage stage;
    vec4 stage_result;

    vec4 prev   = tev_state.color[0];
    vec4 color0 = tev_state.color[1];
    vec4 color1 = tev_state.color[2];
    vec4 color2 = tev_state.color[3];
    vec4 tex;
    vec4 konst;
    vec4 ras;
    vec4 reg_a;
    vec4 reg_b;
    vec4 reg_c;
    vec4 reg_d;
    vec4 scale;
    
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
    prev = fract(_FSDEF_STAGE_DEST * 255.0f/256.0f) * 256.0f/255.0f;
    col0 = prev;
    col1 = prev;
    
    // Alpha compare
    // -------------

    int val = int(prev.a * 255.0f) & 0xFF;                                   
    if (_FSDEF_ALPHA_COMPARE(val, tev_state.alpha_func_ref0, tev_state.alpha_func_ref1)) {
        discard;
    }
#ifdef _FSDEF_SET_DESTINATION_ALPHA
    col0.a = tev_state.dest_alpha;
#endif
    col0 = _FSDEF_EFB_FORMAT(col0);
}