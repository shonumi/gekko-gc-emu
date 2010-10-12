// gx_fifo.h
// (c) 2005,2009 Gekko Team

#ifndef __FIFODECODER_H__
#define __FIFODECODER_H__

////////////////////////////////////////////////////////////////////////////////
// FIFO MACROS

// gp opcode handling
#define GP_OPMASK(n)				((n >> 3) & 0x1f)
#define GP_DECLARE(name)			optype __fastcall						\
									GPOPCODE_##name(cgxlist * _gxlist)
#define GP_OPCODE(name)				optype __fastcall						\
									gx_fifo::GPOPCODE_##name(cgxlist * _gxlist)
#define GP_SETOP(n, op)				exec_op[n] = op
#define GP_SETOPSIZE(n, size)		command_size[GP_OPMASK(n)] = size

// gp fifo handling
#define FIFO_SIZE					(1024 * 16)	// 16kb - good size
#define FIFO_MASK					(SIZE_OF_FIFO - 1)	// mask

// gx indexed xf load type
#define GX_IDX_A					0
#define GX_IDX_B					1
#define GX_IDX_C					2
#define GX_IDX_D					3

// gx packet descriptors, used to send primitive data to the video core
enum gx_packet_descriptions
{
	GXP_NOP							= 0x00,
	GXP_LOAD_CP_REG					= 0x08,
	GXP_LOAD_XF_REG					= 0x10,
	GXP_LOAD_IDX_A					= 0x20,
	GXP_LOAD_IDX_B					= 0x28,
	GXP_LOAD_IDX_C					= 0x30,
	GXP_LOAD_IDX_D					= 0x38,
	GXP_CALL_DISPLAYLIST			= 0x40,
	GXP_INVALIDATE_VERTEX_CACHE		= 0x48,
	GXP_LOAD_BP_REG					= 0x60,
	GXP_DRAW_QUADS					= 0x80,
	GXP_DRAW_TRIANGLES				= 0x90,
	GXP_DRAW_TRIANGLESTRIP			= 0x98,
	GXP_DRAW_TRIANGLEFAN			= 0xA0,
	GXP_DRAW_LINES					= 0xA8,
	GXP_DRAW_LINESTRIP				= 0xB0,
	GXP_DRAW_POINTS					= 0xB8
};

////////////////////////////////////////////////////////////////////////////////
// GENERIC GX COMMAND LIST CLASS

class cgxlist
{
private:
	u32		size;
	u32		mask;
	u32		ptr_put;
	u32		ptr_pop;
	u32		buffer_inram;

public:

	u32		bytes_in_list;
	u8*		buffer;
 
	__inline cgxlist::cgxlist()
	{
		ptr_put = ptr_pop = bytes_in_list = 0;
	}

	__inline void set_size( u32 _size, u8* _ptr, u32 InRAM )
	{	
		ptr_put = ptr_pop = bytes_in_list = 0;

		// make size a power of 2..
		size = _size;

		// point buffer to dlist location..
		buffer = _ptr;
		buffer_inram = InRAM;
	}
	
	__inline void empty(void)
	{
		ptr_put = ptr_pop = bytes_in_list = 0;
		memset(buffer, 0, size);
	}

	__inline u8 get_size(void) { return bytes_in_list; }

	__inline void put8(u8 n)
	{
		buffer[ptr_put] = n;

		ptr_put++;
		bytes_in_list++;
	}

	__inline void put16(u16 n)
	{
		*(u16 *)(&buffer[ptr_put]) = BSWAP16(n);

		ptr_put+=2;
		bytes_in_list+=2;
	}
 
	__inline void put32(u32 n)
	{
		*(u32 *)(&buffer[ptr_put]) = BSWAP32(n);

		ptr_put+=4;
		bytes_in_list+=4;
	}
 
	__inline u8 pop8(void)
	{
		u8 ret;
		if(buffer_inram)
		{
			u32 ret2 = ((u32)buffer + ptr_pop) & RAM_MASK;
			ret = Mem_RAM[(ret2+0) ^ 3];
		}
		else
			ret = buffer[ptr_pop];

		ptr_pop++;
		bytes_in_list--;
 
		return ret;
	}
 
	__inline u16 pop16(void)
	{
		u16 ret;

		if(buffer_inram)
		{
			u32 ret2 = ((u32)buffer + ptr_pop) & RAM_MASK;
			ret = (Mem_RAM[(ret2+0) ^ 3] << 8) |
				  (Mem_RAM[(ret2+1) ^ 3]);
		}
		else
			ret = BSWAP16(*(u16 *)(&buffer[ptr_pop]));

		ptr_pop+=2;
		bytes_in_list-=2;
 
		return ret;
	}
 
	__inline u32 pop32(void)
	{
		u32 ret;
		if(buffer_inram)
		{
			ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			ret = (Mem_RAM[(ret+0) ^ 3] << 24) |
				  (Mem_RAM[(ret+1) ^ 3] << 16) |
				  (Mem_RAM[(ret+2) ^ 3] << 8) |
				  (Mem_RAM[(ret+3) ^ 3]);
		}
		else
			ret = BSWAP32(*(u32 *)(&buffer[ptr_pop]));

		ptr_pop+=4;
		bytes_in_list-=4;
		return ret;
	}

	__inline void pop8_16(u8 *pop1, u16 *pop2)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			*pop1 = Mem_RAM[(ret+0) ^ 3];
			*pop2 = ((u16)Mem_RAM[(ret+1) ^ 3] << 8) |
					((u16)Mem_RAM[(ret+2) ^ 3]);
		}
		else
		{
			*pop1 = buffer[ptr_pop];
			*pop2 = BSWAP16(*(u16 *)(&buffer[ptr_pop+1]));
		}

		ptr_pop+=3;
		bytes_in_list-=3;
		return;
	}

	__inline void pop8_32(u8 *pop1, u32 *pop2)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			*pop1 = Mem_RAM[(ret+0) ^ 3];
			*pop2 = ((u32)Mem_RAM[(ret+1) ^ 3] << 24) |
					((u32)Mem_RAM[(ret+2) ^ 3] << 16) |
					((u32)Mem_RAM[(ret+3) ^ 3] << 8) |
					((u32)Mem_RAM[(ret+4) ^ 3]);
		}
		else
		{
			*pop1 = buffer[ptr_pop];
			*pop2 = BSWAP32(*(u32 *)(&buffer[ptr_pop+1]));
		}
		ptr_pop+=5;
		bytes_in_list-=5;
		return;
	}

	__inline void pop8_24(u8 *pop1, u32 *pop2)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			*pop1 = Mem_RAM[(ret+0) ^ 3];
			*pop2 = ((u32)Mem_RAM[(ret+1) ^ 3] << 16) |
					((u32)Mem_RAM[(ret+2) ^ 3] << 8) |
					((u32)Mem_RAM[(ret+3) ^ 3]);
		}
		else
		{
			*pop1 = buffer[ptr_pop];
			*pop2 = BSWAP32(*(u32 *)(&buffer[ptr_pop])) & 0x00FFFFFF;
		}
		ptr_pop+=4;
		bytes_in_list-=4;
		return;
	}

	__inline void pop16_32(u16 *pop1, u32 *pop2)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			*pop1 = ((u16)Mem_RAM[(ret+0) ^ 3] << 8) |
					((u16)Mem_RAM[(ret+1) ^ 3]);
			*pop2 = ((u32)Mem_RAM[(ret+2) ^ 3] << 24) |
					((u32)Mem_RAM[(ret+3) ^ 3] << 16) |
					((u32)Mem_RAM[(ret+4) ^ 3] << 8) |
					((u32)Mem_RAM[(ret+5) ^ 3]);
		}
		else
		{
			*pop1 = BSWAP16(*(u16 *)(&buffer[ptr_pop]));
			*pop2 = BSWAP32(*(u32 *)(&buffer[ptr_pop+2]));
		}
		ptr_pop+=6;
		bytes_in_list-=6;
		return;
	}

	__inline void pop32_32(u32 *pop1, u32 *pop2)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			*pop1 = ((u32)Mem_RAM[(ret+0) ^ 3] << 24) |
					((u32)Mem_RAM[(ret+1) ^ 3] << 16) |
					((u32)Mem_RAM[(ret+2) ^ 3] << 8) |
					((u32)Mem_RAM[(ret+3) ^ 3]);
			*pop2 = ((u32)Mem_RAM[(ret+4) ^ 3] << 24) |
					((u32)Mem_RAM[(ret+5) ^ 3] << 16) |
					((u32)Mem_RAM[(ret+6) ^ 3] << 8) |
					((u32)Mem_RAM[(ret+7) ^ 3]);
		}
		else
		{
			*pop1 = BSWAP32(*(u32 *)(&buffer[ptr_pop]));
			*pop2 = BSWAP32(*(u32 *)(&buffer[ptr_pop+4]));
		}
		ptr_pop+=8;
		bytes_in_list-=8;
		return;
	}

	__inline void popoff(int _size)
	{
		ptr_pop+=_size;
		bytes_in_list-=_size;
	}

	__inline u8 get8(int _start)
	{
		if(buffer_inram)
			return Mem_RAM[(((u32)buffer + ptr_pop + _start) & RAM_MASK) ^ 3];
		else
			return buffer[(ptr_pop + _start)];
	}
 
	__inline u8 top8(void) { return get8(0); }
 
	__inline u8 lastcmd(void) { return buffer[(ptr_put - 5)]; }

	__inline u16 get16(int _start) {
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop + _start) & RAM_MASK;
			return ((u16)Mem_RAM[(ret+0) ^ 3] << 8) |
				   ((u16)Mem_RAM[(ret+1) ^ 3]);
		}
		else
			return BSWAP16(*(u16 *)(&buffer[(ptr_pop + _start)]));
	}

	__inline u16 get16_noswap(int _start) {
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop + _start) & RAM_MASK;
			return ((u16)Mem_RAM[(ret+1) ^ 3] << 8) |
				   ((u16)Mem_RAM[(ret+0) ^ 3]);
		}
		else
			return *(u16 *)(&buffer[(ptr_pop + _start)]);
	}
 
	__inline u16 top16(void) { return get16(0); }
 
	__inline u32 get32(int _start)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop + _start) & RAM_MASK;
			return ((u32)Mem_RAM[(ret+0) ^ 3] << 24) |
				   ((u32)Mem_RAM[(ret+1) ^ 3] << 16) |
				   ((u32)Mem_RAM[(ret+2) ^ 3] << 8)  |
				   ((u32)Mem_RAM[(ret+3) ^ 3]);
		}
		else
			return BSWAP32(*(u32 *)(&buffer[(ptr_pop + _start)]));
	}
 
	__inline u32 get32_noswap(int _start)
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop + _start) & RAM_MASK;
			return ((u32)Mem_RAM[(ret+3) ^ 3] << 24) |
				   ((u32)Mem_RAM[(ret+2) ^ 3] << 16) |
				   ((u32)Mem_RAM[(ret+1) ^ 3] << 8)  |
				   ((u32)Mem_RAM[(ret+0) ^ 3]);
		}
		else
			return *(u32 *)(&buffer[(ptr_pop + _start)]);
	}

	__inline u32 top32() { return get32(0); }

	// fifo only, call after a commands finishes executing..
	__inline void recirculate(void)
	{
		//memcpy(&buffer[0], &buffer[ptr_pop], (FIFO_SIZE - ptr_pop));
		//allow filling our buffer before shifting it. Our buffer is
		//2x the physical buffer so filling is ok
		if((ptr_pop + bytes_in_list) > (8*1024))
		{
 			memcpy(&buffer[0], &buffer[ptr_pop], bytes_in_list);
			ptr_put -= ptr_pop;
			ptr_pop = 0;
		}
	}
 
	__inline void *getbufferptr()
	{
		if(buffer_inram)
		{
			u32 ret = ((u32)buffer + ptr_pop) & RAM_MASK;
			return &Mem_RAM[ret];
		}
		else
			return &buffer[ptr_pop];
	}

	__inline u8& operator[]( int n )
	{ return buffer[n]; }
};

typedef void				optype;
typedef optype				(__fastcall *gptable)(cgxlist * gxlist);

////////////////////////////////////////////////////////////////////////////////
// FIFO AND DISPLAY LIST DECODER

namespace gx_fifo
{
	extern u8		cmd;
	extern u8		vat;

	extern u8*		fifo_buffer;
	extern cgxlist	fifo;

	extern gptable	exec_op[0x20];
	extern u16		command_size[0x1f];

	// gp opcodes
	GP_DECLARE(UNKNOWN);
	GP_DECLARE(NOP);
	GP_DECLARE(LOAD_CP_REG);
	GP_DECLARE(LOAD_XF_REG);
	GP_DECLARE(LOAD_IDX_A);
	GP_DECLARE(LOAD_IDX_B);
	GP_DECLARE(LOAD_IDX_C);
	GP_DECLARE(LOAD_IDX_D);
	GP_DECLARE(CALL_DISPLAYLIST);
	GP_DECLARE(INVALIDATE_VERTEX_CACHE);
	GP_DECLARE(LOAD_BP_REG);
	GP_DECLARE(DRAW_QUADS);
	GP_DECLARE(DRAW_TRIANGLES);
	GP_DECLARE(DRAW_TRIANGLESTRIP);
	GP_DECLARE(DRAW_TRIANGLEFAN);
	GP_DECLARE(DRAW_LINES);
	GP_DECLARE(DRAW_LINESTRIP);
	GP_DECLARE(DRAW_POINTS);

	// command processing
	u8 __fastcall check_size();
	void __fastcall command_parser(cgxlist * gxlist);
	void __fastcall call_displaylist(u32 _addr, u32 _size);

	// fifo access 8bit and 16bit
	static __inline void pop_gx_8_16(cgxlist* _gxlist, u8* _x, u16* _y)
	{
		_gxlist->pop8_16(_x, _y);
	}

	// fifo access 8bit and 24bit
	static __inline void pop_gx_8_24(cgxlist* _gxlist, u8* _x, u32* _y)
	{
		_gxlist->pop8_24(_x, _y);
	}

	// fifo access 8bit and 32bit
	static __inline void pop_gx_8_32(cgxlist* _gxlist, u8* _x, u32* _y)
	{
		_gxlist->pop8_32(_x, _y);
	}

	// fifo access 16bit and 16bit
	static __inline void pop_gx_16_16(cgxlist* _gxlist, u16* _x, u16* _y)
	{
		u32 ret = _gxlist->pop32();
		*_x = ret >> 16;
		*_y = ret & 0xFFFF;
	}

	// fifo access 16bit and 32bit
	static __inline void pop_gx_16_32(cgxlist* _gxlist, u16* _x, u32* _y)
	{
		_gxlist->pop16_32(_x, _y);
	}

	// fifo access 32bit and 32bit
	static __inline void pop_gx_32_32(cgxlist* _gxlist, u32* _x, u32* _y)
	{
		_gxlist->pop32_32(_x, _y);
	}

	// namespace
	void initialize(void);
	void destroy(void);
};

////////////////////////////////////////////////////////////////////////////////
// EOF

#endif