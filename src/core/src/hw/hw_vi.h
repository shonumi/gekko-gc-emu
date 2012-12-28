// hw_vi.h
// (c) 2005,2008 Gekko Team / Wiimu Project

#ifndef _HW_VI_H_
#define _HW_VI_H_

////////////////////////////////////////////////////////////////////////////////

#define REGVI16(X)			(*((u16 *) &VIRegisters[REG_SIZE - (X & REG_MASK) - 2]))
#define REGVI32(X)			(*((u32 *) &VIRegisters[REG_SIZE - (X & REG_MASK) - 4]))

extern u8 VIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////////////////////////

#define VI_NTSC				0
#define VI_PAL				1
#define VI_MPAL				2
#define VI_DEBUG			3

#define VI_NTSC_INTER       525         // 60 Hz
#define VI_NTSC_NON_INTER   263         // 30 Hz
#define VI_PAL_INTER        625         // 50 Hz
#define VI_PAL_NON_INTER    313         // 25 Hz

#define	VI_CR_INTERLACED	0x4

#define VI_VTR				0xCC002000
#define VI_DCR				0xCC002002
#define VI_TFBL				0xCC00201C
#define VI_BFBL				0xCC002024
#define VI_DPV				0xCC00202C
#define VI_DPH				0xCC00202E
#define VI_DI0				0xCC002030
#define VI_DI1				0xCC002034
#define VI_DI2				0xCC002038
#define VI_DI3				0xCC00203C

#define VI_VICLK			0xCC00206C //NEW

#define VI_DI_INT			0x80000000
#define VI_DI_ENB			0x10000000

////////////////////////////////////////////////////////////////////////////////

#define VI_SCANLINE		REGVI16(VI_DPV)

////////////////////////////////////////////////////////////////////////////////

#define FB_WIDTH		640
#define FB_HEIGHT		480
#define FB_YUYV			4

#define BCLAMP(res) (u8)( (res > 0xFF) ? 255 : ( (res < 0) ? 0 : res ) )

#define VI_DI_VER(x)	( ( x >> 10 ) & 0x3ff )
#define VI_DI_HOZ(x)	( x & 0x3ff )

////////////////////////////////////////////////////////////////////////////////

typedef struct t_sVI
{
	u16		format;			// TV format
	u16		framerate;		// Format Framerate
	u16		vretrace;		// Lines Per Frame
	u32		tickcount;		// Ticks per frame
	u16		vct[4];			// Vertical Interrupt Position
	u64		timer;			// Timer for Cycles left

	u32		xfb_addr;		// Address of the external frame buffer.

	u32		is_interlaced;	// Interlaced Mode
	bool	is_xfb;			// Frame Buffer Mode
	bool	is_autosync;	// Used for new demos

	u8*		xfbbuf;			// Pointer to XFB
	u8		fb_data[(480 * 640 * 4)];
}sVI;

extern sVI vi;

////////////////////////////////////////////////////////////////////////////////

void VI_Open(void);
void VI_Update(void);

void VI_YCbCr2RGB(void);

u8		EMU_FASTCALL	VI_Read8(u32 addr);
void	EMU_FASTCALL	VI_Write8(u32 addr, u32 data);
u16		EMU_FASTCALL	VI_Read16(u32 addr);
void	EMU_FASTCALL	VI_Write16(u32 addr, u32 data);
u32		EMU_FASTCALL	VI_Read32(u32 addr);
void	EMU_FASTCALL	VI_Write32(u32 addr, u32 data);

////////////////////////////////////////////////////////////////////////////////

#endif
