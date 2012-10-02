// gx_tev.h
// (c) 2005,2009 Gekko Team

#ifndef __gx_tev_H__
#define __gx_tev_H__

////////////////////////////////////////////////////////////////////////////////
// TEV MACROS

#define MAX_TEV_STAGES				16
#define MAX_SHADERCACHE_SIZE		256
#define MAX_TEV_PROGRAM				16384

#define GX_TEV_KCSEL_1					0x00
#define GX_TEV_KCSEL_7_8				0x01
#define GX_TEV_KCSEL_3_4				0x02
#define GX_TEV_KCSEL_5_8				0x03
#define GX_TEV_KCSEL_1_2				0x04
#define GX_TEV_KCSEL_3_8				0x05
#define GX_TEV_KCSEL_1_4				0x06
#define GX_TEV_KCSEL_1_8				0x07
#define GX_TEV_KCSEL_K0					0x0C
#define GX_TEV_KCSEL_K1					0x0D
#define GX_TEV_KCSEL_K2					0x0E
#define GX_TEV_KCSEL_K3					0x0F
#define GX_TEV_KCSEL_K0_R				0x10
#define GX_TEV_KCSEL_K1_R				0x11
#define GX_TEV_KCSEL_K2_R				0x12
#define GX_TEV_KCSEL_K3_R				0x13
#define GX_TEV_KCSEL_K0_G				0x14
#define GX_TEV_KCSEL_K1_G				0x15
#define GX_TEV_KCSEL_K2_G				0x16
#define GX_TEV_KCSEL_K3_G				0x17
#define GX_TEV_KCSEL_K0_B				0x18
#define GX_TEV_KCSEL_K1_B				0x19
#define GX_TEV_KCSEL_K2_B				0x1A
#define GX_TEV_KCSEL_K3_B				0x1B
#define GX_TEV_KCSEL_K0_A				0x1C
#define GX_TEV_KCSEL_K1_A				0x1D
#define GX_TEV_KCSEL_K2_A				0x1E
#define GX_TEV_KCSEL_K3_A				0x1F

////////////////////////////////////////////////////////////////////////////////
// TEV EMULATION

// tev negate
static const char *tev_sub[2] = 
{
	"+",
	"-", 
};

// tev bias
static const char *tev_bias[4] = 
{
	"",
	"+ 0.5", 
	"- 0.5",
	""
};

// tev scale
static const char *tev_scale[4] = 
{
	"1.0*",
	"2.0*", 
	"4.0*",
	"0.5*"
};

// tev color op format
static const char *tev_colorop_input[16] = 
{
	"cc_cprev.rgb", 
	"(cc_cprev.a * cc_one.rgb)",
	"cc_c0.rgb",
	"(cc_c0.a * cc_one.rgb)",
	"cc_c1.rgb",
	"(cc_c1.a * cc_one.rgb)",
	"cc_c2.rgb",
	"(cc_c2.a * cc_one.rgb)",
	"cc_texc.rgb",
	"(cc_texc.a * cc_one.rgb)",
	"cc_rasc.rgb",
	"(cc_rasc.a * cc_one.rgb)",
	"cc_one.rgb",
	"cc_half.rgb",
	"konst.rgb",
	"cc_zero.rgb"
};

// tev color out
static const char *tev_colorop_output[4] = 
{
	"cc_cprev.rgb",
	"cc_c0.rgb",
	"cc_c1.rgb",
	"cc_c2.rgb",
};

// tev out
static const char *tev_output[4] = 
{
	"cc_cprev",
	"cc_c0",
	"cc_c1",
	"cc_c2",
};

// tev konst color format
static const char *tev_konst_colorop[32] = 
{
	"1.0 * cc_one.rgb",
	"0.875 * cc_one.rgb",
	"0.75 * cc_one.rgb",
	"0.625 * cc_one.rgb",
	"0.5 * cc_one.rgb",
	"0.375 * cc_one.rgb",
	"0.25 * cc_one.rgb",
	"0.125 * cc_one.rgb",
	"ERROR",
	"ERROR",
	"ERROR",
	"ERROR",
	"kc_c0.rgb",
	"kc_c1.rgb",
	"kc_c2.rgb",
	"kc_c3.rgb",
	"kc_c0.r * cc_one.rgb",
	"kc_c1.r * cc_one.rgb",
	"kc_c2.r * cc_one.rgb",
	"kc_c3.r * cc_one.rgb",
	"kc_c0.g * cc_one.rgb",
	"kc_c1.g * cc_one.rgb",
	"kc_c2.g * cc_one.rgb",
	"kc_c3.g * cc_one.rgb",
	"kc_c0.b * cc_one.rgb",
	"kc_c1.b * cc_one.rgb",
	"kc_c2.b * cc_one.rgb",
	"kc_c3.b * cc_one.rgb",
	"kc_c0.a * cc_one.rgb",
	"kc_c1.a * cc_one.rgb",
	"kc_c2.a * cc_one.rgb",
	"kc_c3.a * cc_one.rgb",
};

// tev alpha op format input
static const char *tev_alphaop_input[8] = 
{
	"cc_cprev.a", 
	"cc_c0.a",
	"cc_c1.a",
	"cc_c2.a",
	"cc_texc.a",
	"cc_rasc.a",
	"konst.a",
	"0.0"
};

// tev alpha op format output
static const char *tev_alphaop_output[4] = 
{
	"cc_cprev.a",
	"cc_c0.a",
	"cc_c1.a",
	"cc_c2.a",
};

// tev konst alpha format
static const char *tev_konst_alphaop[32] = 
{
	"1.0",
	"0.875",
	"0.75",
	"0.625",
	"0.5",
	"0.375",
	"0.25",
	"0.125",
	"ERROR",
	"ERROR",
	"ERROR",
	"ERROR",
	"ERROR",
	"ERROR",
	"ERROR",
	"ERROR",
	"kc_c0.r",
	"kc_c1.r",
	"kc_c2.r",
	"kc_c3.r",
	"kc_c0.g",
	"kc_c1.g",
	"kc_c2.g",
	"kc_c3.g",
	"kc_c0.b",
	"kc_c1.b",
	"kc_c2.b",
	"kc_c3.b",
	"kc_c0.a",
	"kc_c1.a",
	"kc_c2.a",
	"kc_c3.a",
};

////////////////////////////////////////////////////////////////////////////////
//  TEV STRUCTURES

// tev cache shader entry
struct tev_cache_entry
{
	u32	combiner[32];				// color/alpha combiner ops
	u32	konst[16];					// constant color selectors
	f32	kc[4][4];					// color registers
	u32	cmode1;						// destination alpha
	GLhandleARB vp, fp, program;	// shader program
};

////////////////////////////////////////////////////////////////////////////////
// TEV EMULATOR NAMESPACE

namespace gx_tev
{
	struct cache_data
	{
		GLhandleARB		program;
		GLint			u_cc[4];
		GLint			u_t[8];
	};

	// cache variables
	extern bool				is_modified;
	extern int				cache_count;
	extern u32				cache_table[MAX_SHADERCACHE_SIZE];
	extern cache_data		cache_program[MAX_SHADERCACHE_SIZE];

	// tev variables
	extern int				active_stages;
	extern int				active_texs;
	extern int				active_colors;

	// tev registers
	extern u32				combiner[32];
	extern f32				r_cc[4][4];
	extern f32				r_kc[4][4];

	// tev uniform variables
	extern GLint			u_cc[4];
	extern GLint			u_kc[4];

	// tev functions
	u32		get_hash(void);
	void	upload_color(u8 _addr, u32 _value);

	// cache functions
	void	set_modifed(void);
	int		check_cache_entry(u32 _hash);
	void	add_to_cache(void);

	// shader functions
	void	generate_shader(void);
	void	generate_vertex_program(void);
	void	generate_fragment_program(void);

	// namespace
	void initialize(void);
	void destroy(void);
};

////////////////////////////////////////////////////////////////////////////////
// EOF

#endif
