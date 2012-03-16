// hw_gx.h
// (c) 2005,2008 Gekko Team / Wiimu Project

#ifndef HW_GX_H_
#define HW_GX_H_

//

////////////////////////////////////////////////////////////////////////////////
// BP REGISTER DECODING
////////////////////////////////////////////////////////////////////////////////

// GEN MODE

typedef struct _BPGenMode{
	union{
		struct{
			unsigned ntex : 4;
			unsigned ncol : 5;
			unsigned ms_en : 1;
			unsigned ntev : 4;
			unsigned reject_en : 2;
			unsigned nbmp : 3;
			unsigned zfreeze : 5;
			//unsigned rid : 8;
		};
		u32 _u32;
	};
}GenMode;

// PE CMode 1

typedef struct _BPCMode1{
	union{
		struct{
			unsigned alpha : 8;
			unsigned enable : 1;
			unsigned pad0 : 15;
			unsigned rid : 8;
		};
		u32 _u32;
	};

	f32 getalpha(){ return alpha / 255.0f; }
}CMode1;

// TEV color / alpha combiners 

typedef struct _BPTevCombiner{
	union {
		struct{
			unsigned seld : 4;
			unsigned selc : 4;
			unsigned selb : 4;
			unsigned sela : 4;
			unsigned bias : 2;
			unsigned sub : 1;
			unsigned clamp : 1;
			unsigned shift : 2;
			unsigned dest : 2;
			unsigned rid : 8;
		};
		u32 _u32;
	}color;

	union {
		struct{
			unsigned rswap : 2;
			unsigned tswap : 2;
			unsigned seld : 3;
			unsigned selc : 3;
			unsigned selb : 3;
			unsigned sela : 3;
			unsigned bias : 2;
			unsigned sub : 1;
			unsigned clamp : 1;
			unsigned shift : 2;
			unsigned dest : 2;
			unsigned rid : 8;
		};
		u32 _u32;
	}alpha;
}TevCombiner;

// TEV konstant color/alpha selector

typedef struct _BPTevKSel{
	union{
		struct{
			unsigned xrb : 2;
			unsigned xga : 2;
			unsigned kcsel0 : 5;
			unsigned kasel0 : 5;
			unsigned kcsel1 : 5;
			unsigned kasel1 : 5;
			unsigned rid : 8;
		};
		u32 _u32;
	};

	int getkc(int stage){ return (stage&1) ? kcsel1 : kcsel0; }
	int getka(int stage){ return (stage&1) ? kasel1 : kasel0; }
}TevKSel;

// TEV raster color order

typedef struct _BPTevOrder{
	union{
		struct{
			unsigned texmap0 : 3;
			unsigned texcoord0 : 3;
			unsigned texmapenable0 : 1;
			unsigned colorid0 : 3;
			unsigned pad0 : 2;
			unsigned texmap1 : 3;
			unsigned texcoord1 : 3;
			unsigned texmapenable1 : 1;
			unsigned colorid1 : 3;
			unsigned pad1 : 2;
			unsigned rid : 8;
		};
		u32 _u32;
	};

	inline int get_texmap(int stage){ return (stage&1) ? texmap1 : texmap0; }
	inline int get_texcoord(int stage){ return (stage&1) ? texcoord1 : texcoord0; }
	inline int get_enable(int stage){ return (stage&1) ? texmapenable1 : texmapenable0; }
	inline int get_colorchan(int stage){ return (stage&1) ? colorid1 : colorid0; }
}TevOrder;

////////////////////////////////////////////////////////////////////////////////

union BPMemory{
	struct{
		GenMode			genmode;
		u32				pad0[0x27];
		TevOrder		tevorder[0x8];
		u32				pad1[0x12];
		CMode1			cmode1;
		u32				pad2[0x7D];
		TevCombiner		combiner[0x10];
		u32				pad3[0x16];
		TevKSel			ksel[0x8];
	};

	u32 mem[0x100];
};

////////////////////////////////////////////////////////////////////////////////
// CP REGISTER DECODING
////////////////////////////////////////////////////////////////////////////////

union CPMemory{
	u32 mem[0x100];
};

////////////////////////////////////////////////////////////////////////////////
// XF REGISTER DECODING
////////////////////////////////////////////////////////////////////////////////

// cp: register reference
#define CP_IDX_ADDR(idx, n)				(cp.mem[0xac + n] + idx * cp.mem[0xbc + n])

// mask the base of an xf address
#define XF_ADDR_MASK					0xff00

////////////////////////////////////////////////////////////////////////////////

// XF Num Colors
typedef struct _XFNumColors{
	u32 n;
}NumColors;

// XF Color
typedef struct _XFColorChannel{
	union{
		struct{
			u8 a, b, g, r;
		};
		u32 _u32;
	};
}ColorChannel;

// XF Color
typedef struct _XFColorControl{
	union{
		struct{
			unsigned materialsrc : 1;
			unsigned lightfunc : 1;
			unsigned light0 : 1;
			unsigned light1 : 1;
			unsigned light2 : 1;
			unsigned light3 : 1;
			unsigned ambientsrc : 1;
			unsigned diffuseatten : 1;
			unsigned attenenable : 1;
			unsigned attenselect : 1;
			unsigned light4 : 1;
			unsigned light5 : 1;
			unsigned light6 : 1;
			unsigned light7 : 1;
			unsigned rid : 18;
		};
		u32 _u32;
	};
}ColorControl;

////////////////////////////////////////////////////////////////////////////////

union XFMemory{
	struct{
		u32				pad0[0x9];
		NumColors		numcolors;
		ColorChannel	ambient[0x2];
		ColorChannel	material[0x2];
		ColorControl	colorcontrol[0x2];
		u32				pad1[0xf0];
	};

	u32 mem[0x100];
};

////////////////////////////////////////////////////////////////////////////////

extern BPMemory bp;
extern CPMemory cp;
extern XFMemory xf;
extern u32 xfmem[0x800];

void GX_Open(void);
void GX_Update(void);

void GX_BPLoad(u8 _addr, u32 _value);
void GX_CPLoad(u8 _addr, u32 _value);
void GX_XFLoad(u16 _length, u16 _addr, u32 _regs[64]);
void GX_XFLoadIndexed(u8 _n, u16 _index, u8 _length, u16 _addr);

u8 EMU_FASTCALL GX_Fifo_Read8(u32 addr);
u16 EMU_FASTCALL GX_Fifo_Read16(u32 addr);
u32 EMU_FASTCALL GX_Fifo_Read32(u32 addr);

void EMU_FASTCALL GX_Fifo_Write8(u32 addr, u32 data);
void EMU_FASTCALL GX_Fifo_Write16(u32 addr, u32 data);
void EMU_FASTCALL GX_Fifo_Write32(u32 addr, u32 data);

////////////////////////////////////////////////////////////////////////////////

#endif
