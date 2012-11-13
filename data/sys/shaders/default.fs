// Prepare and compute TEV stage result
#define STAGE_RESULT(s) \
    tex = __PREDEF_TEXTURE_##s; \
 \
    stage = bp_regs.tev_stages[s]; \
    konst = stage.konst; \
 \
    stage_result = (vec4(__PREDEF_COMBINER_COLOR_D_##s, __PREDEF_COMBINER_ALPHA_D_##s) + \
        (vec4(stage.color_sub, stage.color_sub, stage.color_sub, stage.alpha_sub) * \
        (mix(vec4(__PREDEF_COMBINER_COLOR_A_##s, __PREDEF_COMBINER_ALPHA_A_##s), \
        vec4(__PREDEF_COMBINER_COLOR_B_##s, __PREDEF_COMBINER_ALPHA_B_##s), \
        vec4(__PREDEF_COMBINER_COLOR_C_##s, __PREDEF_COMBINER_ALPHA_C_##s)) + \
        vec4(stage.color_bias, stage.color_bias, stage.color_bias, stage.alpha_bias)))); \
 \
    __PREDEF_COMBINER_COLOR_DEST_##s = __PREDEF_CLAMP_COLOR_##s(stage.color_scale * \
        stage_result.rgb); \
    __PREDEF_COMBINER_ALPHA_DEST_##s = __PREDEF_CLAMP_ALPHA_##s(stage_result.a);
    
// Final TEV stage output
#define STAGE_DEST(s) vec4(__PREDEF_COMBINER_COLOR_DEST_##s, __PREDEF_COMBINER_ALPHA_DEST_##s);
    
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

layout(std140) uniform BPRegisters {
    TevState tev_state;
    TevStage tev_stages[16];
} bp_regs;

// Textures
uniform sampler2D texture[8];
in vec4 vtx_color_0;
in vec2 vtx_texcoord[8];

out vec4 frag_dest;

void main() {
    float alpha; 
    TevStage stage;
    vec4 stage_result;

    vec4 prev = bp_regs.tev_state.color[0];
    vec4 color0= bp_regs.tev_state.color[1];
    vec4 color1 = bp_regs.tev_state.color[2];
    vec4 color2 = bp_regs.tev_state.color[3];
    vec4 tex;
    vec4 konst;
    vec4 ras = vtx_color_0;
    
    STAGE_RESULT(0);
#if __PREDEF_NUM_STAGES > 0
    STAGE_RESULT(1);
#endif
#if __PREDEF_NUM_STAGES > 1
    STAGE_RESULT(2);
#endif
#if __PREDEF_NUM_STAGES > 2
    STAGE_RESULT(3);
#endif
#if __PREDEF_NUM_STAGES > 3
    STAGE_RESULT(4);
#endif
#if __PREDEF_NUM_STAGES > 4
    STAGE_RESULT(5);
#endif
#if __PREDEF_NUM_STAGES > 5
    STAGE_RESULT(6);
#endif
#if __PREDEF_NUM_STAGES > 6
    STAGE_RESULT(7);
#endif
#if __PREDEF_NUM_STAGES > 7
    STAGE_RESULT(8);
#endif
#if __PREDEF_NUM_STAGES > 8
    STAGE_RESULT(9);
#endif
#if __PREDEF_NUM_STAGES > 9
    STAGE_RESULT(10);
#endif
#if __PREDEF_NUM_STAGES > 10
    STAGE_RESULT(11);
#endif
#if __PREDEF_NUM_STAGES > 11
    STAGE_RESULT(12);
#endif
#if __PREDEF_NUM_STAGES > 12
    STAGE_RESULT(13);
#endif
#if __PREDEF_NUM_STAGES > 13
    STAGE_RESULT(14);
#endif
#if __PREDEF_NUM_STAGES > 14
    STAGE_RESULT(15);
#endif

    // Store result of last TEV stage
    frag_dest = STAGE_DEST(__PREDEF_NUM_STAGES);
    
    // Alpha compare
    // -------------

    int val = int(frag_dest.a * 255.0f) & 0xFF;                                   
    if (!__PREDEF_ALPHA_COMPARE(val, bp_regs.tev_state.alpha_func_ref0, 
        bp_regs.tev_state.alpha_func_ref1))
        discard;
}
