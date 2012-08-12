#version 330

#define BP_REG_GENMODE          0   // 0x00
#define BP_REG_PE_ZMODE         64  // 0x40
#define BP_REG_PE_CONTROL       67  // 0x43
#define BP_REG_TEV_COLOR_ENV    192 // 0xC0
#define BP_REG_TEV_ALPHA_ENV    193 // 0xC1
#define BP_REG_TEV_REGISTER_L   224 // 0xE0
#define BP_REG_TEV_REGISTER_H   225 // 0xE1
#define BP_REG_ALPHACOMPARE     243 // 0xF3
#define BP_REG_TEV_KSEL         246 // 0xF6

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

// Textures
uniform int tex_enable[8];
uniform sampler2D texture0;

// BP memory
uniform vec4    bp_tev_color[4];
uniform vec4    bp_tev_konst[4];
uniform int     bp_mem[0x100];

in vec4 vertexColor;
in vec2 vertexTexCoord0;
out vec4 fragmentColor;

// Texture
vec4 g_tex = texture2D(texture0, vertexTexCoord0);

// TEV internal colors
vec4 g_color[4] = {
    bp_tev_color[0],
    bp_tev_color[1],
    bp_tev_color[2],
    bp_tev_color[3]
};

// TEV color constants
vec3 tev_kc[32] = {
    vec3(1.0, 1.0, 1.0),
    vec3(0.875, 0.875, 0.875),
    vec3(0.75, 0.75, 0.75),
    vec3(0.625, 0.625, 0.625),
    vec3(0.5, 0.5, 0.5),
    vec3(0.375, 0.375, 0.375),
    vec3(0.25, 0.25, 0.25),
    vec3(0.125, 0.125, 0.125),
    vec3(0.0, 0.0, 0.0),            // undefined
    vec3(0.0, 0.0, 0.0),            // undefined
    vec3(0.0, 0.0, 0.0),            // undefined
    vec3(0.0, 0.0, 0.0),            // undefined
    bp_tev_konst[0].rgb,
    bp_tev_konst[1].rgb,
    bp_tev_konst[2].rgb,
    bp_tev_konst[3].rgb,
    bp_tev_konst[0].rrr,
    bp_tev_konst[1].rrr,
    bp_tev_konst[2].rrr,
    bp_tev_konst[3].rrr,
    bp_tev_konst[0].ggg,
    bp_tev_konst[1].ggg,
    bp_tev_konst[2].ggg,
    bp_tev_konst[3].ggg,
    bp_tev_konst[0].bbb,
    bp_tev_konst[1].bbb,
    bp_tev_konst[2].bbb,
    bp_tev_konst[3].bbb,
    bp_tev_konst[0].aaa,
    bp_tev_konst[1].aaa,
    bp_tev_konst[2].aaa,
    bp_tev_konst[3].aaa,
};

// TEV combiner functions
// TODO(ShizZy): Verify these are actually right...
const float tev_scale[4] = { 
    1.0, 2.0, 4.0, 0.5
};
const float tev_sub[2] = { 
    1.0, -1.0 
};
const vec3 tev_bias[4] = {
    vec3(0.0, 0.0, 0.0), 
    vec3(0.5, 0.5, 0.5), 
    vec3(-0.5, -0.5, -0.5), 
    vec3(0.0, 0.0, 0.0) 
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

vec3 tev_stage(in int stage) {

    int reg_color_env = bp_mem[BP_REG_TEV_COLOR_ENV + (stage << 1)];

    int bias = (reg_color_env >> 16) & 0x3;
    int sub = (reg_color_env >> 18) & 0x1;
    int clamp = (reg_color_env >> 19) & 0x1;
    int shift = (reg_color_env >> 20) & 0x3;
    int dest = (reg_color_env >> 22) & 0x3;
    int kcsel0 = (bp_mem[BP_REG_TEV_KSEL] >> 4) & 0x1F;
    vec3 konst = tev_kc[kcsel0];
    
    
    vec3 tev_cc[16] = {
        g_color[0].rgb,
        g_color[0].aaa,
        g_color[1].rgb,
        g_color[1].aaa,
        g_color[2].rgb,
        g_color[2].aaa,
        g_color[3].rgb,
        g_color[3].aaa,
        g_tex.rgb,
        g_tex.aaa,
        vertexColor.rgb,
        vertexColor.aaa,
        vec3(1.0f, 1.0f, 1.0f),
        vec3(0.5f, 0.5f, 0.5f),
        konst.rgb,
        vec3(0.0f, 0.0f, 0.0f),
    };
    
    vec3 cc_d = tev_cc[reg_color_env & 0xF];
    vec3 cc_c = tev_cc[(reg_color_env >> 4) & 0xF];
    vec3 cc_b = tev_cc[(reg_color_env >> 8) & 0xF];
    vec3 cc_a = tev_cc[(reg_color_env >> 12) & 0xF];
    
    g_color[dest].rgb = tev_scale[shift] * (cc_d + (tev_sub[sub] * (mix(cc_a, cc_b, cc_c) + tev_bias[bias])));
    
    if (clamp == 1) g_color[dest].rgb = clamp(g_color[dest].rgb, 0.0, 1.0);
    
    return g_color[dest].rgb;
}

void main() {
    vec4 dest;

    if (tex_enable[0] == 1) {
        dest = g_tex * vertexColor;
    } else {
        dest = vertexColor;
    }
    
    // TEV stages
    // ----------
    
    int num_tevstages = ((bp_mem[BP_REG_GENMODE] >> 10) & 0xF) + 1;

    if (num_tevstages > 0) dest.rgb = tev_stage(0);
    if (num_tevstages > 1) dest.rgb = tev_stage(1);
    if (num_tevstages > 2) dest.rgb = tev_stage(2);
    if (num_tevstages > 3) dest.rgb = tev_stage(3);
    if (num_tevstages > 4) dest.rgb = tev_stage(4);
    if (num_tevstages > 5) dest.rgb = tev_stage(5);
    if (num_tevstages > 6) dest.rgb = tev_stage(6);
    if (num_tevstages > 7) dest.rgb = tev_stage(7);
    if (num_tevstages > 8) dest.rgb = tev_stage(8);
    if (num_tevstages > 9) dest.rgb = tev_stage(9);
    if (num_tevstages > 10) dest.rgb = tev_stage(10);
    if (num_tevstages > 11) dest.rgb = tev_stage(11);
    if (num_tevstages > 12) dest.rgb = tev_stage(12);
    if (num_tevstages > 13) dest.rgb = tev_stage(13);
    if (num_tevstages > 14) dest.rgb = tev_stage(14);
    if (num_tevstages > 15) dest.rgb = tev_stage(15);

    // Alpha compare
    // -------------
    
    int a_op0 = (bp_mem[BP_REG_ALPHACOMPARE] >> 16) & 0x7;
    int a_op1 = (bp_mem[BP_REG_ALPHACOMPARE] >> 19) & 0x7;
    int a_ref0 = (bp_mem[BP_REG_ALPHACOMPARE] & 0xFF);
    int a_ref1 = ((bp_mem[BP_REG_ALPHACOMPARE] >> 8) & 0xFF);
    int logic_op = ((bp_mem[BP_REG_ALPHACOMPARE] >> 22) & 0x3);
    int val = int(dest.a * 255.0f) & 0xFF;
    
    switch (logic_op) {
    case GX_AOP_AND:
        if (!(alpha_compare(a_op0, val, a_ref0) && alpha_compare(a_op1, val, a_ref1))) discard;
        break;
    case GX_AOP_OR:
        if (!(alpha_compare(a_op0, val, a_ref0) || alpha_compare(a_op1, val, a_ref1))) discard;
        break;
    case GX_AOP_XOR:
        if (!(alpha_compare(a_op0, val, a_ref0) != alpha_compare(a_op1, val, a_ref1))) discard;
        break;
    case GX_AOP_XNOR:
        if (!(alpha_compare(a_op0, val, a_ref0) == alpha_compare(a_op1, val, a_ref1))) discard;
        break;
    }
    
    // ZComploc
    // --------
    
    /*
    int z_comploc = (bp_mem[BP_REG_PE_CONTROL] >> 6) & 1;
    int z_update_enable = (bp_mem[BP_REG_PE_ZMODE] >> 4) & 1;
    
    if (z_comploc == 0 && z_update_enable == 0) {
        discard;
    }
    */
   
    fragmentColor = dest;
}
