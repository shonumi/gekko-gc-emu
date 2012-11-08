#define GX_NEVER    0
#define GX_LESS     1
#define GX_EQUAL    2
#define GX_LEQUAL   3
#define GX_GREATER  4
#define GX_NEQUAL   5
#define GX_GEQUAL   6
#define GX_ALWAYS   7

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
uniform sampler2D texture[8];

in vec4 vtx_color_0;
in vec2 vtx_texcoord[8];

out vec4 frag_dest;

vec4 g_color[16] = vec4[16](
    bp_regs.tev_state.color[0].rgba,
    bp_regs.tev_state.color[0].aaaa,
    bp_regs.tev_state.color[1].rgba,
    bp_regs.tev_state.color[1].aaaa,
    bp_regs.tev_state.color[2].rgba,
    bp_regs.tev_state.color[2].aaaa,
    bp_regs.tev_state.color[3].rgba,
    bp_regs.tev_state.color[3].aaaa,
    vec4(0.0f, 0.0f, 0.0f, 0.0f), // g_tex.rgba,
    vec4(0.0f, 0.0f, 0.0f, 0.0f), // g_tex.aaaa,
    vtx_color_0.rgba,
    vtx_color_0.aaaa,
    vec4(1.0f, 1.0f, 1.0f, 1.0f),
    vec4(0.5f, 0.5f, 0.5f, 0.5f),
    vec4(0.0f, 0.0f, 0.0f, 0.0f), // konst - set dynamically
    vec4(0.0f, 0.0f, 0.0f, 0.0f)
);

bool alpha_compare(in int op, in int value, in int ref) {
    switch (op) {
    case GX_NEVER:
        return false;
    case GX_LESS:
        return (value < ref);
    case GX_EQUAL:
        return (value == ref);
    case GX_LEQUAL:
        return (value <= ref);
    case GX_GREATER:
        return (value > ref);
    case GX_NEQUAL:
        return (value != ref);
    case GX_GEQUAL:
        return (value >= ref);
    case GX_ALWAYS:
        return true;
    }
    return true;
}

void StageResult(in int stage_index) {
    TevStage stage = bp_regs.tev_stages[stage_index];

    // Update konst register
    g_color[14].rgb = stage.konst.rgb; // Note 'a' is used for alpha konst zero value
    g_color[12].a = stage.konst.a;
    
    // Texture
    if (stage.texture_enable != 0) {
        g_color[8] = texture2D(texture[stage.texture_map], vtx_texcoord[stage.texture_coords]);
        g_color[9] = vec4(g_color[8].a, g_color[8].a, g_color[8].a, g_color[8].a);
    } else {
        g_color[8] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        g_color[9] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    vec4 tev_input_a = vec4(g_color[stage.color_sel_a].rgb, g_color[stage.alpha_sel_a].a);
    vec4 tev_input_b = vec4(g_color[stage.color_sel_b].rgb, g_color[stage.alpha_sel_b].a);
    vec4 tev_input_c = vec4(g_color[stage.color_sel_c].rgb, g_color[stage.alpha_sel_c].a);
    vec4 tev_input_d = vec4(g_color[stage.color_sel_d].rgb, g_color[stage.alpha_sel_d].a);
    vec4 sub = vec4(stage.color_sub, stage.color_sub, stage.color_sub, stage.alpha_sub);
    vec4 bias = vec4(stage.color_bias, stage.color_bias, stage.color_bias, stage.alpha_bias);

    // Process stage
    vec4 result = (tev_input_d + (sub * (mix(tev_input_a, tev_input_b, tev_input_c) + bias)));

    // Clamp color
    if (stage.color_clamp == 1) {
        g_color[stage.color_dest].rgb = 
            clamp(stage.color_scale * result.rgb, 0.0, 1.0);
    } else {
        g_color[stage.color_dest].rgb = stage.color_scale * result.rgb;
    }

    // Clamp alpha
    float alpha;
    if (stage.alpha_clamp == 1) {
        alpha = clamp(result.a, 0.0, 1.0);
    } else {
        alpha = result.a;
    }
    g_color[stage.alpha_dest << 1].a = alpha;
    g_color[(stage.alpha_dest << 1) + 1] = vec4(alpha, alpha, alpha, alpha);
}

void main() {
    StageResult(0);
#if NUM_STAGES > 1
    StageResult(1);
#endif
#if NUM_STAGES > 2
    StageResult(2);
#endif
#if NUM_STAGES > 3
    StageResult(3);
#endif
#if NUM_STAGES > 4
    StageResult(4);
#endif
#if NUM_STAGES > 5
    StageResult(5);
#endif
#if NUM_STAGES > 6
    StageResult(6);
#endif
#if NUM_STAGES > 7
    StageResult(7);
#endif
#if NUM_STAGES > 8
    StageResult(8);
#endif
#if NUM_STAGES > 9
    StageResult(9);
#endif
#if NUM_STAGES > 10
    StageResult(10);
#endif
#if NUM_STAGES > 11
    StageResult(11);
#endif
#if NUM_STAGES > 12
    StageResult(12);
#endif
#if NUM_STAGES > 13
    StageResult(13);
#endif
#if NUM_STAGES > 14
    StageResult(14);
#endif
#if NUM_STAGES > 15
    StageResult(15);
#endif

    // Store result of last TEV stage
    vec4 dest = vec4(g_color[bp_regs.tev_stages[NUM_STAGES - 1].color_dest].rgb, 
        g_color[bp_regs.tev_stages[NUM_STAGES - 1].alpha_dest].a);

    // Alpha compare
    // -------------

    int val = int(dest.a * 255.0f) & 0xFF;

#ifdef BP_ALPHA_FUNC_AND
    if (!(alpha_compare(bp_regs.tev_state.alpha_func_comp0, val, bp_regs.tev_state.alpha_func_ref0) && 
        alpha_compare(bp_regs.tev_state.alpha_func_comp1, val, bp_regs.tev_state.alpha_func_ref1)))
            discard;
#elif defined(BP_ALPHA_FUNC_OR)
    if (!(alpha_compare(bp_regs.tev_state.alpha_func_comp0, val, bp_regs.tev_state.alpha_func_ref0) || 
        alpha_compare(bp_regs.tev_state.alpha_func_comp1, val, bp_regs.tev_state.alpha_func_ref1)))
            discard;
#elif defined(BP_ALPHA_FUNC_XOR)
    if (!(alpha_compare(bp_regs.tev_state.alpha_func_comp0, val, bp_regs.tev_state.alpha_func_ref0) != 
        alpha_compare(bp_regs.tev_state.alpha_func_comp1, val, bp_regs.tev_state.alpha_func_ref1)))
            discard;
#elif defined(BP_ALPHA_FUNC_XNOR)
    if (!(alpha_compare(bp_regs.tev_state.alpha_func_comp0, val, bp_regs.tev_state.alpha_func_ref0) == 
        alpha_compare(bp_regs.tev_state.alpha_func_comp1, val, bp_regs.tev_state.alpha_func_ref1)))
            discard;
#endif

    frag_dest = dest;
}
