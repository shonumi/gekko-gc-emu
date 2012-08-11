#version 150

#define BP_REG_PE_ZMODE         64  // 0x40
#define BP_REG_PE_CONTROL       67  // 0x43
#define BP_REG_ALPHACOMPARE     243 // 0xF3

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
uniform int    bp_mem[0x100];

in vec4 vertexColor;
in vec2 vertexTexCoord0;
out vec4 fragmentColor;


bool col_compare(in int op, in int value, in int ref) {
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

void main() {
    vec4 col0;

    if (tex_enable[0] == 1) {
        col0 = texture2D(texture0, vertexTexCoord0) * vertexColor;
    } else {
        col0 = vertexColor;
    }
    
    // Alpha compare
    // -------------
    
    int a_op0 = (bp_mem[BP_REG_ALPHACOMPARE] >> 16) & 0x7;
    int a_op1 = (bp_mem[BP_REG_ALPHACOMPARE] >> 19) & 0x7;
    int a_ref0 = (bp_mem[BP_REG_ALPHACOMPARE] & 0xFF);
    int a_ref1 = ((bp_mem[BP_REG_ALPHACOMPARE] >> 8) & 0xFF);
    int logic_op = ((bp_mem[BP_REG_ALPHACOMPARE] >> 22) & 0x3);
    int val = int(col0.a * 255.0f) & 0xFF;
    
    switch (logic_op) {
    case GX_AOP_AND:
        if (!(col_compare(a_op0, val, a_ref0) && col_compare(a_op1, val, a_ref1))) discard;
        break;
    case GX_AOP_OR:
        if (!(col_compare(a_op0, val, a_ref0) || col_compare(a_op1, val, a_ref1))) discard;
        break;
    case GX_AOP_XOR:
        if (!(col_compare(a_op0, val, a_ref0) != col_compare(a_op1, val, a_ref1))) discard;
        break;
    case GX_AOP_XNOR:
        if (!(col_compare(a_op0, val, a_ref0) == col_compare(a_op1, val, a_ref1))) discard;
        break;
    }
    /*
    // ZComploc
    // --------
    
    int z_comploc = (bp_mem[BP_REG_PE_CONTROL] >> 6) & 1;
    int z_update_enable = (bp_mem[BP_REG_PE_ZMODE] >> 4) & 1;
    
    if (z_comploc == 0 && z_update_enable == 0) {
        discard;
    }
    */
    fragmentColor = col0;
}
