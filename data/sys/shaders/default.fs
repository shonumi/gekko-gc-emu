#version 140
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_uniform_buffer_object : enable

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

struct BPTevStage {
    int color_sel_a;
    int color_sel_b;
    int color_sel_c;
    int color_sel_d;
    int color_bias;
    int color_sub;
    int color_clamp;
    int color_shift;
    int color_dest;

    int alpha_sel_a;
    int alpha_sel_b;
    int alpha_sel_c;
    int alpha_sel_d;
    int alpha_bias;
    int alpha_sub;
    int alpha_clamp;
    int alpha_shift;
    int alpha_dest;

    // These don't really need to be here, but it's a reminder that this struct must be padded to a
    // 16 byte boundary in order to be tightly packed in the UBO
    int pad0;
    int pad1;
};

layout(std140) uniform BPRegisters {
   BPTevStage tev_stage[16];
} bp_regs;

// Textures
uniform int tex_enable[8];
uniform sampler2D texture0;

// BP memory
uniform vec4    bp_tev_color[4];
uniform vec4    bp_tev_konst[4];

// BP_REG_GENMODE - 0x00
uniform int     bp_genmode_num_stages;

// BP_REG_TEV_COLOR_ENV - 0xC0-
uniform int bp_tev_color_env[128];
//uniform int bp_tev_alpha_env[144];
uniform int bp_tev_ksel[32];

// BP_REG_ALPHACOMPARE - 0xF3
uniform int     bp_alpha_func_ref0;
uniform int     bp_alpha_func_ref1;
uniform int     bp_alpha_func_comp0;
uniform int     bp_alpha_func_comp1;
uniform int     bp_alpha_func_logic;

in vec4 vertexColor;
in vec2 vertexTexCoord0;
out vec4 fragmentColor;

// Texture
vec4 g_tex = texture2D(texture0, vertexTexCoord0);

vec4 g_color[] = vec4[](
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
);

// TEV color constants
vec4 tev_konst[32] = vec4[](
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
    bp_tev_konst[3].aaaa
);

// TEV combiner functions
// TODO(ShizZy): Verify these are actually right...
float tev_scale[4] = float[](
    1.0, 2.0, 4.0, 0.5
);
float tev_sub[2] = float[](
    1.0, -1.0
);
float tev_bias[4] = float[](
    0.0, 0.5, -0.5, 0.0
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

void tev_stage(in int stage) {

    int color_sel = bp_tev_ksel[(stage << 1) + 0]; // should index stage?
    int alpha_sel = bp_tev_ksel[(stage << 1) + 1]; // should index stage?

    BPTevStage tev_stage = bp_regs.tev_stage[stage];

    // Update konst register
    g_color[14].rgb = tev_konst[color_sel].rgb;
    g_color[12].a = tev_konst[alpha_sel].a;

    vec4 tev_input_a = vec4(g_color[tev_stage.color_sel_a].rgb, 
        g_color[(tev_stage.alpha_sel_a << 1)].a);
    vec4 tev_input_b = vec4(g_color[tev_stage.color_sel_b].rgb, 
        g_color[(tev_stage.alpha_sel_b << 1)].a);
    vec4 tev_input_c = vec4(g_color[tev_stage.color_sel_c].rgb, 
        g_color[(tev_stage.alpha_sel_c << 1)].a);
    vec4 tev_input_d = vec4(g_color[tev_stage.color_sel_d].rgb, 
        g_color[(tev_stage.alpha_sel_d << 1)].a);
    vec4 sub = vec4(tev_sub[tev_stage.color_sub], tev_sub[tev_stage.color_sub], 
        tev_sub[tev_stage.color_sub], tev_sub[tev_stage.alpha_sub]);
    vec4 bias = vec4(tev_bias[tev_stage.color_bias], tev_bias[tev_stage.color_bias], 
        tev_bias[tev_stage.color_bias], tev_bias[tev_stage.alpha_bias]);

    // Process stage
    vec4 result = (tev_input_d + (sub * (mix(tev_input_a, tev_input_b, tev_input_c) + bias)));

    // Clamp color
    if (tev_stage.color_clamp == 1) {
        g_color[tev_stage.color_dest].rgb = 
            clamp(tev_scale[tev_stage.color_shift] * result.rgb, 0.0, 1.0);
    } else {
        g_color[tev_stage.color_dest].rgb = tev_scale[tev_stage.color_shift] * result.rgb;
    }

    // Clamp alpha
    float alpha;
    if (tev_stage.alpha_clamp == 1) {
        alpha = clamp(result.a, 0.0, 1.0);
    } else {
        alpha = result.a;
    }
    g_color[tev_stage.alpha_dest << 1].a = alpha;
    g_color[(tev_stage.alpha_dest << 1) + 1] = vec4(alpha, alpha, alpha, alpha);
}

void main() {

    vec4 dest;

    // TEV stages
    // ----------    

    if (bp_genmode_num_stages > 0)  { tev_stage(0);
    if (bp_genmode_num_stages > 1)  { tev_stage(1);
    if (bp_genmode_num_stages > 2)  { tev_stage(2);
    if (bp_genmode_num_stages > 3)  { tev_stage(3);
    if (bp_genmode_num_stages > 4)  { tev_stage(4);
    if (bp_genmode_num_stages > 5)  { tev_stage(5);
    if (bp_genmode_num_stages > 6)  { tev_stage(6);
    if (bp_genmode_num_stages > 7)  { tev_stage(7);
    if (bp_genmode_num_stages > 8)  { tev_stage(8);
    if (bp_genmode_num_stages > 9)  { tev_stage(9);
    if (bp_genmode_num_stages > 10) { tev_stage(10);
    if (bp_genmode_num_stages > 11) { tev_stage(11);
    if (bp_genmode_num_stages > 12) { tev_stage(12);
    if (bp_genmode_num_stages > 13) { tev_stage(13);
    if (bp_genmode_num_stages > 14) { tev_stage(14);
    if (bp_genmode_num_stages > 15) { tev_stage(15);
    }}}}}}}}}}}}}}}
    // Store result of last TEV stage
    dest = vec4(g_color[bp_regs.tev_stage[bp_genmode_num_stages].color_dest].rgb, 
        g_color[bp_regs.tev_stage[bp_genmode_num_stages].alpha_dest].a);
    }

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
