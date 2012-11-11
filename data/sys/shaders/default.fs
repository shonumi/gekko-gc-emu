////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE The following definitions interface to the dynamically generated preprocessor code, which 
// are prefaced by the __PREDEF tag. Do not use directly, instead create an interface macro here to
// clearly distinguish between generated code and non-generated code

#define NUM_STAGES                      __PREDEF_NUM_STAGES
#define ALPHA_COMPARE(val, ref0, ref1)  __PREDEF_ALPHA_COMPARE(val, ref0, ref1)

////////////////////////////////////////////////////////////////////////////////////////////////////

// Prepare and compute TEV stage result
#define STAGE_RESULT(s) \
    if (bp_regs.tev_stages[s].texture_enable != 0) { \
        g_color[8] = texture2D(texture[s], vtx_texcoord[bp_regs.tev_stages[s].texture_coords]); \
        g_color[9] = g_color[8].aaaa; \
    } else { \
        g_color[8] = vec4(1.0f, 1.0f, 1.0f, 1.0f); \
        g_color[9] = vec4(1.0f, 1.0f, 1.0f, 1.0f); \
    } \
    g_color[12].a = bp_regs.tev_stages[s].konst.a; \
    g_color[14].rgb = bp_regs.tev_stages[s].konst.rgb; \
    stage = bp_regs.tev_stages[s]; \
 \
    stage_result = (vec4(g_color[stage.color_sel_d].rgb, g_color[stage.alpha_sel_d].a) + \
        (vec4(stage.color_sub, stage.color_sub, stage.color_sub, stage.alpha_sub) * \
        (mix(vec4(g_color[stage.color_sel_a].rgb, g_color[stage.alpha_sel_a].a), \
        vec4(g_color[stage.color_sel_b].rgb, g_color[stage.alpha_sel_b].a), \
        vec4(g_color[stage.color_sel_c].rgb, g_color[stage.alpha_sel_c].a)) + \
        vec4(stage.color_bias, stage.color_bias, stage.color_bias, stage.alpha_bias)))); \
 \
    if (stage.color_clamp == 1) { \
        g_color[stage.color_dest].rgb =  clamp(stage.color_scale * stage_result.rgb, 0.0, 1.0); \
    } else { \
        g_color[stage.color_dest].rgb = stage.color_scale * stage_result.rgb; \
    } \
 \
    if (stage.alpha_clamp == 1) { \
        alpha = clamp(stage_result.a, 0.0, 1.0); \
    } else { \
        alpha = stage_result.a; \
    } \
    g_color[stage.alpha_dest << 1].a = alpha; \
    g_color[(stage.alpha_dest << 1) + 1] = vec4(alpha, alpha, alpha, alpha);

struct TevStage {
    int color_sel_a;
    int color_sel_b;
    int color_sel_c;
    int color_sel_d;
    float color_bias;
    float color_sub;
    int color_clamp;
    float color_scale;
    int color_dest;

    int alpha_sel_a;
    int alpha_sel_b;
    int alpha_sel_c;
    int alpha_sel_d;
    float alpha_bias;
    float alpha_sub;
    int alpha_clamp;
    float alpha_scale;
    int alpha_dest;

    int texture_enable;
    int texture_map;
    int texture_coords;
    
    int pad0;
    int pad1;
    int pad2;
    
    vec4 konst;

    // NOTE: this struct must be padded to a 16 byte boundary in order to be tightly packed
};

struct TevState {
    int alpha_func_ref0;
    int alpha_func_ref1;
    int alpha_func_comp0;
    int alpha_func_comp1;

    int pad0;
    int pad1;
    int pad2;
    int pad3;
    
    vec4 color[4];

    // NOTE: this struct must be padded to a 16 byte boundary in order to be tightly packed
};

layout(std140) uniform BPRegisters {
    TevState tev_state;
    TevStage tev_stages[16];
} bp_regs;

// Textures
uniform sampler2D texture[16];

in vec4 vtx_color_0;
in vec2 vtx_texcoord[8];

out vec4 frag_dest;

// This is setup for stage 0, but certain parts are updated with each stage - mainly, konst and
// texture indices
vec4 g_color[16] = vec4[16](
    bp_regs.tev_state.color[0],
    bp_regs.tev_state.color[0].aaaa,
    bp_regs.tev_state.color[1],
    bp_regs.tev_state.color[1].aaaa,
    bp_regs.tev_state.color[2],
    bp_regs.tev_state.color[2].aaaa,
    bp_regs.tev_state.color[3],
    bp_regs.tev_state.color[3].aaaa,
    vec4(1.0f, 1.0f, 1.0f, 1.0f), // tex
    vec4(1.0f, 1.0f, 1.0f, 1.0f), // tex.aaaa
    vtx_color_0,
    vtx_color_0.aaaa,
    vec4(1.0f, 1.0f, 1.0f, 1.0f), // alpha is konst color - set dynamically
    vec4(0.5f, 0.5f, 0.5f, 0.5f),
    vec4(0.0f, 0.0f, 0.0f, 0.0f), // konst RGB - set dynamically
    vec4(0.0f, 0.0f, 0.0f, 0.0f)
);

void main() {
    float alpha; 
    TevStage stage;
    vec4 stage_result;
    
    STAGE_RESULT(0);
#if NUM_STAGES > 1
    STAGE_RESULT(1);
#endif
#if NUM_STAGES > 2
    STAGE_RESULT(2);
#endif
#if NUM_STAGES > 3
    STAGE_RESULT(3);
#endif
#if NUM_STAGES > 4
    STAGE_RESULT(4);
#endif
#if NUM_STAGES > 5
    STAGE_RESULT(5);
#endif
#if NUM_STAGES > 6
    STAGE_RESULT(6);
#endif
#if NUM_STAGES > 7
    STAGE_RESULT(7);
#endif
#if NUM_STAGES > 8
    STAGE_RESULT(8);
#endif
#if NUM_STAGES > 9
    STAGE_RESULT(9);
#endif
#if NUM_STAGES > 10
    STAGE_RESULT(10);
#endif
#if NUM_STAGES > 11
    STAGE_RESULT(11);
#endif
#if NUM_STAGES > 12
    STAGE_RESULT(12);
#endif
#if NUM_STAGES > 13
    STAGE_RESULT(13);
#endif
#if NUM_STAGES > 14
    STAGE_RESULT(14);
#endif
#if NUM_STAGES > 15
    STAGE_RESULT(15);
#endif

    // Store result of last TEV stage
    frag_dest = vec4(g_color[bp_regs.tev_stages[NUM_STAGES - 1].color_dest].rgb, 
        g_color[bp_regs.tev_stages[NUM_STAGES - 1].alpha_dest].a);

    // Alpha compare
    // -------------

    int val = int(frag_dest.a * 255.0f) & 0xFF;                                   
    if (!ALPHA_COMPARE(val, bp_regs.tev_state.alpha_func_ref0, bp_regs.tev_state.alpha_func_ref1))
        discard;
}
