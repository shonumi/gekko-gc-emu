// gx_fifo.cpp
// (c) 2005,2009 Gekko Team

#include "common.h"
#include "memory.h"
#include "opengl.h"
#include "hw/hw_gx.h"
#include "gx_fifo.h"
#include "gx_vertex.h"

////////////////////////////////////////////////////////////////////////////////

u8		gx_fifo::cmd;
u8		gx_fifo::vat;

u8*		gx_fifo::fifo_buffer;
cgxlist	gx_fifo::fifo;

gptable	gx_fifo::exec_op[0x20];
u16		gx_fifo::command_size[0x1f];

/* The following is a display list cache hack.  
		Could be utilized better on say static DL's ?

typedef struct{
	GLuint dlist;
	int n;
	BPMemory cache_bp;
	CPMemory cache_cp;
	XFMemory cache_xf;
}DLCache;

DLCache dl_cache[0x10000];

GP_OPCODE (CALL_DISPLAYLIST)
{
	u32 addr, size;
	u16 c_addr;

	pop_gx_32_32(_gxlist, &addr, &size);

	c_addr = (addr&0xffff)^(addr>>16);

	if(!dl_cache[c_addr].n<10) // if ready to recreate list
	{
		dl_cache[c_addr].n = 50; // every 50 times a dl is called, recreate it
		glNewList(dl_cache[c_addr].dlist,GL_COMPILE); // create new list
		call_displaylist(addr, size); // decode data
		glEndList(); // end list

		// perform a full register cache
		memcpy(&dl_cache[c_addr].cache_bp, &bp, sizeof(BPMemory));
		memcpy(&dl_cache[c_addr].cache_cp, &cp, sizeof(CPMemory));
		memcpy(&dl_cache[c_addr].cache_xf, &xf, sizeof(XFMemory));
	}else{
		// otherwise, pull up a cached list and registers
		memcpy(&bp, &dl_cache[c_addr].cache_bp, sizeof(BPMemory));
		memcpy(&cp, &dl_cache[c_addr].cache_cp, sizeof(CPMemory));
		memcpy(&xf, &dl_cache[c_addr].cache_xf, sizeof(XFMemory));
		glCallList(dl_cache[c_addr].dlist);
		dl_cache[c_addr].n--;
	}

	// reset dl cache

	/*memset(&dl_cache[0], 0, 0x10000*sizeof(DLCache));

	for(i=0;i<0x10000;i++)
	{
		dl_cache[i].dlist = glGenLists(1);
	}

}*/

////////////////////////////////////////////////////////////////////////////////
// GRAPHICS PROCESSOR OPCODES

// unknown command
GP_OPCODE (UNKNOWN)
{
	printf("ERROR: GP Fifo has been corrupted.\n");
}

// nop - do nothing
GP_OPCODE (NOP)
{
}

// load cp register with data
GP_OPCODE (LOAD_CP_REG)
{
	u8 addr;
	u32 value;

	pop_gx_8_32(_gxlist, &addr, &value);
	GX_CPLoad(addr, value);
}

// load xf register with data
GP_OPCODE (LOAD_XF_REG)
{
	u16 length, addr;
	u32 regs[64];

	pop_gx_16_16(_gxlist, &length, &addr);
	length++; // length is always set -1

	for(int i = 0; i < length; i++)
		regs[i] = _gxlist->pop32();

	GX_XFLoad(length, addr, regs);
}

// load xf register with data indexed A
GP_OPCODE (LOAD_IDX_A)
{
	u8 length;
	u16 index, data, addr;
	
	pop_gx_16_16(_gxlist, &index, &data);

	length = (data >> 12) + 1;
	addr = data & 0xfff;

	GX_XFLoadIndexed(GX_IDX_A, index, length, addr);
}

// load xf register with data indexed B
GP_OPCODE (LOAD_IDX_B)
{
	u8 length;
	u16 index, data, addr;
	
	pop_gx_16_16(_gxlist, &index, &data);

	length = (data >> 12) + 1;
	addr = data & 0xfff;

	GX_XFLoadIndexed(GX_IDX_B, index, length, addr);
}

// load xf register with data indexed C
GP_OPCODE (LOAD_IDX_C)
{
	u8 length;
	u16 index, data, addr;
	
	pop_gx_16_16(_gxlist, &index, &data);

	length = (data >> 12) + 1;
	addr = data & 0xfff;

	GX_XFLoadIndexed(GX_IDX_C, index, length, addr);
}

// load xf register with data indexed D
GP_OPCODE (LOAD_IDX_D)
{
	u8 length;
	u16 index, data, addr;
	
	pop_gx_16_16(_gxlist, &index, &data);

	length = (data >> 12) + 1;
	addr = data & 0xfff;

	GX_XFLoadIndexed(GX_IDX_D, index, length, addr);
}


// call a display list
GP_OPCODE (CALL_DISPLAYLIST)
{
	u32 addr, size;

	pop_gx_32_32(_gxlist, &addr, &size);

	call_displaylist(addr, size);
}

// invalidate vertex cache
GP_OPCODE (INVALIDATE_VERTEX_CACHE)
{
}

// load bp register with data
GP_OPCODE (LOAD_BP_REG)
{
	u8 addr;
	u32 value;

	pop_gx_8_24(_gxlist, &addr, &value);
	GX_BPLoad(addr, value);
}

// draw a primitive - quads
GP_OPCODE (DRAW_QUADS)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_QUADS, count, vat);
}

// draw a primitive - triangles
GP_OPCODE (DRAW_TRIANGLES)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_TRIANGLES, count, vat);
}

// draw a primitive - trianglestrip
GP_OPCODE (DRAW_TRIANGLESTRIP)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_TRIANGLE_STRIP, count, vat);
}

// draw a primitive - trianglefan
GP_OPCODE (DRAW_TRIANGLEFAN)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_TRIANGLE_FAN, count, vat);
}

// draw a primitive - lines
GP_OPCODE (DRAW_LINES)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_LINES, count, vat);
}

// draw a primitive - linestrip
GP_OPCODE (DRAW_LINESTRIP)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_LINE_STRIP, count, vat);
}

// draw a primitive - points
GP_OPCODE (DRAW_POINTS)
{
	u16 count = _gxlist->pop16();

	gx_vertex::draw_primitive(_gxlist, GL_POINTS, count, vat);
}

////////////////////////////////////////////////////////////////////////////////
// MAIN FIFO CONTROL

// parse a fifo command
void EMU_FASTCALL gx_fifo::command_parser(cgxlist * gxlist)
{
	cmd = gxlist->pop8();
	vat = cmd & 0x7;

	exec_op[GP_OPMASK(cmd)](gxlist);
}

// execute a fifo command
u8 EMU_FASTCALL gx_fifo::check_size()
{
	static u32 LastRequiredSize = -1;

	if((LastRequiredSize != -1) && (LastRequiredSize > fifo.bytes_in_list))
		return 0;

	u8 cmd = fifo.get8(0);
	u8 _vat = cmd & 0x7;

	switch(GP_OPMASK(cmd))
	{
	case 0:
		LastRequiredSize = -1;
		return 1;
		break; // NOP

	case 1:
		if(fifo.bytes_in_list >= 6)
		{
			LastRequiredSize = -1;
			return 1;
		}
		LastRequiredSize = 6;
		break; // LOAD CP

	case 2: // LOAD XF
		if(fifo.bytes_in_list >= 5) // if header is present
		{
			u16 size = (4 * (fifo.get16(1) & 0xf) + 1);
			size+=5;
			if(fifo.bytes_in_list >= size)
			{
				LastRequiredSize = -1;
				return 1;
			}
			else
				LastRequiredSize = size;
		}
		else
			LastRequiredSize = 5;

		break;

	case 4: // LOAD IDX A
	case 5: // " B
	case 6: // " C 
	case 7:
		if(fifo.bytes_in_list >= 5)
		{
			LastRequiredSize = -1;
			return 1;
		}
		LastRequiredSize = 5;
		break; // " D

	case 8:
		if(fifo.bytes_in_list >= 9)
		{
			LastRequiredSize = -1;
			return 1;
		}
		LastRequiredSize = 9;
		break; // CALL DL

	case 9:
		LastRequiredSize = -1;
		return 1;
		break; // INVALID VTX CACHE

	case 0xC:
		if(fifo.bytes_in_list >= 5)
		{
			LastRequiredSize = -1;
			return 1;
		}
		LastRequiredSize = 5;
		break; // LOAD BP

	default: 		
		if(cmd & 0x80) // draw command
		{
			if(fifo.bytes_in_list >= 3)	//see if header exists
			{
				u16 numverts = fifo.get16(1);
				u16 vertsize = gx_vertex::get_size(_vat);
				u16 size = 3;

				size += numverts * vertsize;
				if(fifo.bytes_in_list >= size)
				{
					LastRequiredSize = -1;
					return 1;
				}
				else
					LastRequiredSize = size;
			}
			else
				LastRequiredSize = 3;
		}
		break;
	}

	return 0;
}

// execute a display list
void EMU_FASTCALL gx_fifo::call_displaylist(u32 _addr, u32 _size)
{
	cgxlist dl;
	u32 maddr = _addr & 0xffff;

	dl.set_size(_size, (u8 *)_addr, 1);
	dl.bytes_in_list = _size;

	while(dl.bytes_in_list > 0)
	{
		command_parser(&dl);
	}
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

void gx_fifo::initialize(void)
{
	// alloc and clear buffer
	fifo_buffer = (u8*)malloc(FIFO_SIZE);
	memset(fifo_buffer, 0, FIFO_SIZE);

	// create fifo
	fifo.set_size(FIFO_SIZE, fifo_buffer, 0);

	// init op table
	for(int i = 0; i < 0x20; i++)
		GP_SETOP(i, GPOPCODE_UNKNOWN);

	// create op table
	GP_SETOP(GP_OPMASK(GXP_NOP), GPOPCODE_NOP);	
	GP_SETOP(GP_OPMASK(GXP_LOAD_CP_REG), GPOPCODE_LOAD_CP_REG);				
	GP_SETOP(GP_OPMASK(GXP_LOAD_XF_REG), GPOPCODE_LOAD_XF_REG);
	GP_SETOP(GP_OPMASK(GXP_LOAD_IDX_A), GPOPCODE_LOAD_IDX_A);
	GP_SETOP(GP_OPMASK(GXP_LOAD_IDX_B), GPOPCODE_LOAD_IDX_B);
	GP_SETOP(GP_OPMASK(GXP_LOAD_IDX_C), GPOPCODE_LOAD_IDX_C);
	GP_SETOP(GP_OPMASK(GXP_LOAD_IDX_D), GPOPCODE_LOAD_IDX_D);
	GP_SETOP(GP_OPMASK(GXP_CALL_DISPLAYLIST), GPOPCODE_CALL_DISPLAYLIST);
	GP_SETOP(GP_OPMASK(GXP_INVALIDATE_VERTEX_CACHE), GPOPCODE_INVALIDATE_VERTEX_CACHE);
	GP_SETOP(GP_OPMASK(GXP_LOAD_BP_REG), GPOPCODE_LOAD_BP_REG);	
	GP_SETOP(GP_OPMASK(GXP_DRAW_QUADS), GPOPCODE_DRAW_QUADS);
	GP_SETOP(GP_OPMASK(GXP_DRAW_TRIANGLES), GPOPCODE_DRAW_TRIANGLES);
	GP_SETOP(GP_OPMASK(GXP_DRAW_TRIANGLESTRIP), GPOPCODE_DRAW_TRIANGLESTRIP);
	GP_SETOP(GP_OPMASK(GXP_DRAW_TRIANGLEFAN), GPOPCODE_DRAW_TRIANGLEFAN);
	GP_SETOP(GP_OPMASK(GXP_DRAW_LINES), GPOPCODE_DRAW_LINES);
	GP_SETOP(GP_OPMASK(GXP_DRAW_LINESTRIP), GPOPCODE_DRAW_LINESTRIP);
	GP_SETOP(GP_OPMASK(GXP_DRAW_POINTS), GPOPCODE_DRAW_POINTS);

	// create op size table, note arbitrary h4ck0rz
	GP_SETOPSIZE(GXP_NOP, 1);
	GP_SETOPSIZE(GXP_LOAD_CP_REG, 6);
	GP_SETOPSIZE(GXP_LOAD_XF_REG, 64);				// arbitrary
	GP_SETOPSIZE(GXP_LOAD_IDX_A, 5);
	GP_SETOPSIZE(GXP_LOAD_IDX_B, 5);
	GP_SETOPSIZE(GXP_LOAD_IDX_C, 5);
	GP_SETOPSIZE(GXP_LOAD_IDX_D, 5);
	GP_SETOPSIZE(GXP_CALL_DISPLAYLIST, 9);
	GP_SETOPSIZE(GXP_INVALIDATE_VERTEX_CACHE, 1);
	GP_SETOPSIZE(GXP_LOAD_BP_REG, 5);
	GP_SETOPSIZE(GXP_DRAW_QUADS, 64);				// arbitrary
	GP_SETOPSIZE(GXP_DRAW_TRIANGLES, 64);			// arbitrary
	GP_SETOPSIZE(GXP_DRAW_TRIANGLESTRIP, 64);		// arbitrary
	GP_SETOPSIZE(GXP_DRAW_TRIANGLEFAN, 64);			// arbitrary
	GP_SETOPSIZE(GXP_DRAW_LINES, 64);				// arbitrary
	GP_SETOPSIZE(GXP_DRAW_LINESTRIP, 64);			// arbitrary
	GP_SETOPSIZE(GXP_DRAW_POINTS, 64);				// arbitrary
}

void gx_fifo::destroy(void)
{
	free(fifo_buffer);
	fifo_buffer = 0;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
