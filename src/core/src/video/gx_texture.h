// gx_texture.h
// (c) 2005,2009 Gekko Team

#ifndef __gx_texture_H__
#define __gx_texture_H__

////////////////////////////////////////////////////////////////////////////////
// TEXTURE FORMAT DECODER MACROS

#define TMEM_SIZE						0x100000
#define TMEM_MASK						0x0fffff
#define TEX_CACHE_LOCATION(n)				((n & 0xffff) ^ (n >> 16))
#define PTEXTURE_MEM(addr)				&gx_texture::tmem[addr & TMEM_MASK]

////////////////////////////////////////////////////////////////////////////////
// TEXTURE FORMAT DECODER STRUCTURES

// texture base data structure
struct gx_texture_data
{
	u8		fmt;		// format
	u8		num;		// num (0->8)
	u32		addr;		// address
	u16		height;		// height dimension
	u16		width;		// width dimension
	u16		min_filter;	// filter (min)
	u16		mag_filter;	// filter (mag)
	u16		wrap_t;		// clamp/repeat/mirror t
	u16		wrap_s;		// clamp/repeat/mirror s
};

// Structure for the TGA texture format (for dumping)

typedef struct {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
} TgaHeader;

////////////////////////////////////////////////////////////////////////////////
// TEXTURE FORMAT DECODER NAMESPACE

namespace gx_texture
{
	// texture memory
	extern u8 tmem[TMEM_SIZE];

	// texture control
	void add(gx_texture_data _tx);

	// utilities
	void dump_to_tga(char [], u16, u16, u8*);

	// namespace
	void initialize(void);
	void reset(void);
	void destroy(void);
};

////////////////////////////////////////////////////////////////////////////////

#endif

