#version 150

#define GX_NEVER    0
#define GX_LESS     1
#define GX_EQUAL    2
#define GX_LEQUAL   3
#define GX_GREATER  4
#define GX_NEQUAL   5
#define GX_GEQUAL   6
#define GX_ALWAYS   7

#define GX_AOP_AND  0
#define GX_AOP_OR   1
#define GX_AOP_XOR  2
#define GX_AOP_XNOR 3

#define GX_TEV_BIAS_COMPARE  3

// Textures
uniform int tex_enable[8];
uniform sampler2D texture0;

// BP memory
uniform vec4    bp_tev_color[4];
uniform vec4    bp_tev_konst[4];

// BP_REG_GENMODE - 0x00
uniform int     bp_genmode_num_stages;

struct BPTevStage {
    int sel_a;
    int sel_b;
    int sel_c;
    int sel_d;
    int bias;
    int sub;
    int clamp;
    int shift;
    int dest;
};

struct BPTevKSel {
	int color_sel;
	int alpha_sel;
};

// BP_REG_ALPHACOMPARE - 0xF3
uniform int     bp_alpha_func_ref0;
uniform int     bp_alpha_func_ref1;
uniform int     bp_alpha_func_comp0;
uniform int     bp_alpha_func_comp1;
uniform int     bp_alpha_func_logic;

// BP_REG_TEV_COLOR_ENV - 0xC0-
uniform int		bp_tev_color_env[144];

// BP_REG_TEV_ALPHA_ENV - 0xC1-
uniform int 	bp_tev_alpha_env[144];

// BP_REG_TEV_KSEL - 0xF6-
uniform int 	bp_tev_ksel[32];

in vec4 vertexColor;
in vec2 vertexTexCoord0;
out vec4 fragmentColor;

// Texture
vec4 g_tex = texture2D(texture0, vertexTexCoord0);

vec4 g_color[] = {
    bp_tev_color[0].rgba,
    bp_tev_color[0].aaaa,
    bp_tev_color[1].rgba,
    bp_tev_color[1].aaaa,
    bp_tev_color[2].rgba,
    bp_tev_color[2].aaaa,
    bp_tev_color[3].rgba,
    bp_tev_color[3].aaaa,
    g_tex.rgba,
    g_tex.aaaa,
    vertexColor.rgba,
    vertexColor.aaaa,
    vec4(1.0f, 1.0f, 1.0f, 1.0f),
    vec4(0.5f, 0.5f, 0.5f, 0.5f),
    vec4(0.0f, 0.0f, 0.0f, 0.0f), // konst - set dynamically
    vec4(0.0f, 0.0f, 0.0f, 0.0f)
};

// TEV color constants
vec4 tev_konst[32] = {
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(0.875, 0.875, 0.875, 0.875),
    vec4(0.75, 0.75, 0.75, 0.75),
    vec4(0.625, 0.625, 0.625, 0.625),
    vec4(0.5, 0.5, 0.5, 0.5),
    vec4(0.375, 0.375, 0.375, 0.375),
    vec4(0.25, 0.25, 0.25, 0.25),
    vec4(0.125, 0.125, 0.125, 0.125),
    vec4(0.0, 0.0, 0.0, 0.0),            // undefined
    vec4(0.0, 0.0, 0.0, 0.0),            // undefined
    vec4(0.0, 0.0, 0.0, 0.0),            // undefined
    vec4(0.0, 0.0, 0.0, 0.0),            // undefined
    bp_tev_konst[0].rgba,                // alpha unconfirmed
    bp_tev_konst[1].rgba,                // alpha unconfirmed
    bp_tev_konst[2].rgba,                // alpha unconfirmed
    bp_tev_konst[3].rgba,                // alpha unconfirmed
    bp_tev_konst[0].rrrr,
    bp_tev_konst[1].rrrr,
    bp_tev_konst[2].rrrr,
    bp_tev_konst[3].rrrr,
    bp_tev_konst[0].gggg,
    bp_tev_konst[1].gggg,
    bp_tev_konst[2].gggg,
    bp_tev_konst[3].gggg,
    bp_tev_konst[0].bbbb,
    bp_tev_konst[1].bbbb,
    bp_tev_konst[2].bbbb,
    bp_tev_konst[3].bbbb,
    bp_tev_konst[0].aaaa,
    bp_tev_konst[1].aaaa,
    bp_tev_konst[2].aaaa,
    bp_tev_konst[3].aaaa,
};

// TEV combiner functions
// TODO(ShizZy): Verify these are actually right...
const float tev_scale[4] = { 
    1.0, 2.0, 4.0, 0.5
};
const float tev_sub[2] = { 
    1.0, -1.0 
};
const vec4 tev_bias[4] = {
    vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.5, 0.5, 0.5, 0.5),
    vec4(-0.5, -0.5, -0.5, -0.5),
    vec4(0.0, 0.0, 0.0, 0.0)
};

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


vec4 tev_stage(in int stage) {
    BPTevKSel tev_ksel;
    
    tev_ksel.color_sel = bp_tev_ksel[(stage << 1) + 0]; // should index stage?
    tev_ksel.alpha_sel = bp_tev_ksel[(stage << 1) + 1]; // should index stage?
    
    int stage_offset = stage * 9;

    BPTevStage tev_c;
    tev_c.sel_a = bp_tev_color_env[stage_offset + 0];
    tev_c.sel_b = bp_tev_color_env[stage_offset + 1];
    tev_c.sel_c = bp_tev_color_env[stage_offset + 2];
    tev_c.sel_d = bp_tev_color_env[stage_offset + 3];
    tev_c.bias  = bp_tev_color_env[stage_offset + 4];
    tev_c.sub   = bp_tev_color_env[stage_offset + 5];
    tev_c.clamp = bp_tev_color_env[stage_offset + 6];
    tev_c.shift = bp_tev_color_env[stage_offset + 7];
    tev_c.dest  = bp_tev_color_env[stage_offset + 8];

    BPTevStage tev_a;
    tev_a.sel_a = bp_tev_alpha_env[stage_offset + 0];
    tev_a.sel_b = bp_tev_alpha_env[stage_offset + 1];
    tev_a.sel_c = bp_tev_alpha_env[stage_offset + 2];
    tev_a.sel_d = bp_tev_alpha_env[stage_offset + 3];
    tev_a.bias  = bp_tev_alpha_env[stage_offset + 4];
    tev_a.sub   = bp_tev_alpha_env[stage_offset + 5];
    tev_a.clamp = bp_tev_alpha_env[stage_offset + 6];
    tev_a.shift = bp_tev_alpha_env[stage_offset + 7];
    tev_a.dest  = bp_tev_alpha_env[stage_offset + 8];

    // Update konst register
    g_color[14].rgb = tev_konst[tev_ksel.color_sel].rgb;
    g_color[12].a = tev_konst[tev_ksel.alpha_sel].a;

    vec4 tev_input_a = vec4(g_color[tev_c.sel_a].rgb, g_color[(tev_a.sel_a<<1)].a);
    vec4 tev_input_b = vec4(g_color[tev_c.sel_b].rgb, g_color[(tev_a.sel_b<<1)].a);
    vec4 tev_input_c = vec4(g_color[tev_c.sel_c].rgb, g_color[(tev_a.sel_c<<1)].a);
    vec4 tev_input_d = vec4(g_color[tev_c.sel_d].rgb, g_color[(tev_a.sel_d<<1)].a);

    // TODO: Should pre-lookup the values on the CPU and directly input the uniforms in this format
    vec4 scale = vec4(tev_scale[tev_c.shift].rrr, tev_scale[tev_a.shift]);
    vec4 sub = vec4(tev_sub[tev_c.sub].rrr, tev_sub[tev_a.sub]);
    vec4 bias = vec4(tev_bias[tev_c.bias].rgb, tev_bias[tev_a.bias]);

    // Process stage
    vec4 result = scale * (tev_input_d + (sub * (mix(tev_input_a, tev_input_b, tev_input_c) + bias)));
    g_color[tev_c.dest].rgb = result.rgb;

    // Clamp color
    if (tev_c.clamp == 1) g_color[tev_c.dest].rgb = clamp(result.rgb, 0.0, 1.0);
    else g_color[tev_c.dest].rgb = result.rgb;

    // Clamp alpha
    float alpha;
    if (tev_a.clamp == 1) alpha = clamp(result.a, 0.0, 1.0);
    else alpha = result.a;
    g_color[tev_a.dest<<1].a = alpha;
    g_color[(tev_a.dest<<1)+1] = alpha.rrrr;

    return vec4(g_color[tev_c.dest].rgb, alpha);
}

void main() {
    vec4 dest;
   
    // ZComploc
    // --------
    
    /*int z_comploc = (bp_mem[BP_REG_PE_CONTROL] >> 6) & 1;
    int z_update_enable = (bp_mem[BP_REG_PE_ZMODE] >> 4) & 1;
    
    if (z_comploc == 0 && z_update_enable == 0) {
        discard;
    }*/

    // TEV stages
    // ----------    

    if (bp_genmode_num_stages > 0)  { dest = tev_stage(0);
    if (bp_genmode_num_stages > 1)  { dest = tev_stage(1);
    if (bp_genmode_num_stages > 2)  { dest = tev_stage(2);
    if (bp_genmode_num_stages > 3)  { dest = tev_stage(3);
    if (bp_genmode_num_stages > 4)  { dest = tev_stage(4);
    if (bp_genmode_num_stages > 5)  { dest = tev_stage(5);
    if (bp_genmode_num_stages > 6)  { dest = tev_stage(6);
    if (bp_genmode_num_stages > 7)  { dest = tev_stage(7);
    if (bp_genmode_num_stages > 8)  { dest = tev_stage(8);
    if (bp_genmode_num_stages > 9)  { dest = tev_stage(9);
    if (bp_genmode_num_stages > 10) { dest = tev_stage(10);
    if (bp_genmode_num_stages > 11) { dest = tev_stage(11);
    if (bp_genmode_num_stages > 12) { dest = tev_stage(12);
    if (bp_genmode_num_stages > 13) { dest = tev_stage(13);
    if (bp_genmode_num_stages > 14) { dest = tev_stage(14);
    if (bp_genmode_num_stages > 15) { dest = tev_stage(15);
    }}}}}}}}}}}}}}}}

    // Alpha compare
    // -------------
    
    int val = int(dest.a * 255.0f) & 0xFF;
    
    switch (bp_alpha_func_logic) {
    case GX_AOP_AND:
        if (!(alpha_compare(bp_alpha_func_comp0, val, bp_alpha_func_ref0) && 
            alpha_compare(bp_alpha_func_comp1, val, bp_alpha_func_ref1))) discard;
        break;
    case GX_AOP_OR:
        if (!(alpha_compare(bp_alpha_func_comp0, val, bp_alpha_func_ref0) || 
            alpha_compare(bp_alpha_func_comp1, val, bp_alpha_func_ref1))) discard;
        break;
    case GX_AOP_XOR:
        if (!(alpha_compare(bp_alpha_func_comp0, val, bp_alpha_func_ref0) != 
            alpha_compare(bp_alpha_func_comp1, val, bp_alpha_func_ref1))) discard;
        break;
    case GX_AOP_XNOR:
        if (!(alpha_compare(bp_alpha_func_comp0, val, bp_alpha_func_ref0) == 
            alpha_compare(bp_alpha_func_comp1, val, bp_alpha_func_ref1))) discard;
        break;
    }

    fragmentColor = dest;
}

