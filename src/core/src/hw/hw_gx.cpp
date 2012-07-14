// hw_gx.cpp
// (c) 2005,2008 Gekko Team / Wiimu Project

#include "common.h"
#include "memory.h"
#include "hw.h"
#include "hw_gx.h"
#include "hw_pi.h"
#include "hw_pe.h"

#include "video/opengl.h"
#include "video/gx_fifo.h"
#include "video/gx_vertex.h"
#include "video/gx_states.h"
#include "video/gx_vertex.h"
#include "video/gx_tev.h"

#include "video_core.h"
#include "fifo_player.h"

#ifdef USE_NEW_VIDEO_CORE
#include "fifo.h"
#endif

BPMemory bp;
CPMemory cp;
XFMemory xf;
u32 xfmem[0x800];

////////////////////////////////////////////////////////////////////////////////
// GX - Graphics Processor
////////////////////////////////////////////////////////////////////////////////

// Desc: Write data into a BP register
//

void GX_BPLoad(u8 _addr, u32 _value)
{
    //LOG_NOTICE(TGP, "BP_LOAD [%02x] = %08x", _addr, _value);

	// write data to bp memory
	bp.mem[_addr] = _value;

	// adjust gx globals accordingly
	switch(_addr)
	{
	case 0x0: // GEN_MODE
		gx_states::set_cullmode();
		gx_tev::active_stages = bp.genmode.ntev + 1;
		gx_tev::set_modifed();
		break;

	case 0x20: // SU_SCIS0 - Scissorbox Top Left Corner
	case 0x21: // SU_SCIS1 - Scissorbox Bottom Right Corner
		gx_states::set_scissors();
		break;

	case 0x22: // SU_LPSIZE - Line and Point Size
		gx_states::set_lpsize();
		break;

	case 0x28: // RAS_TREF0
	case 0x29: // RAS_TREF1
	case 0x2a: // RAS_TREF2
	case 0x2b: // RAS_TREF3
	case 0x2c: // RAS_TREF4
	case 0x2d: // RAS_TREF5
	case 0x2e: // RAS_TREF6
	case 0x2f: // RAS_TREF7
		gx_tev::set_modifed();
		break;

	case 0x40: // PE_ZMODE
		gx_states::set_zmode();
		break;

	case 0x41: // PE_CMODE0
		gx_states::set_cmode0();
		break;

	case 0x42: // PE_CMODE1
		gx_tev::set_modifed();
		break;

	case 0x45: // PE_DONE - draw done
		//gx_states::draw_done(); //do on write ~ShizZy
		break;

	case 0x47: // PE_TOKEN
		GX_PE_TOKEN_VALUE = (_value & 0xffff); 
		break;

	case 0x48: // PE_TOKEN_INT
		GX_PE_TOKEN_VALUE = (_value & 0xffff); 
		GX_PE_TOKEN = 1; //seems to break mario
		break;

	case 0x4f: // PE copy clear AR - set clear alpha and red components
	case 0x50: // PE copy clear GB - green and blue
		gx_states::set_copyclearcolor();
		break;

	case 0x51: // PE copy clear Z - 24-bit Z value
		gx_states::set_copyclearz();
		break;

	case 0x52: // pe copy execute
		gx_states::copy_efb(); 
		break;

	case 0x59: // Scissorbox Offset
		gx_states::set_scissors();
		break;

	case 0x64: // TX_LOADTLUT0
	case 0x65: // TX_LOADTLUT1
		gx_states::load_tlut();
		break;

	case 0x80: // TX_SETMODE0_I0 - Texture lookup and filtering mode
	case 0x81: // TX_SETMODE0_I1
	case 0x82: // TX_SETMODE0_I2
	case 0x83: // TX_SETMODE0_I3
	case 0xa0: // TX_SETMODE0_I4
	case 0xa1: // TX_SETMODE0_I5
	case 0xa2: // TX_SETMODE0_I6
	case 0xa3: // TX_SETMODE0_I7
		gx_states::tx_setmode0(_addr);
		break;

	case 0xc0: // TEV_COLOR_ENV_0
	case 0xc1: // TEV_ALPHA_ENV_0
	case 0xc2: // TEV_COLOR_ENV_1
	case 0xc3: // TEV_ALPHA_ENV_1
	case 0xc4: // TEV_COLOR_ENV_2
	case 0xc5: // TEV_ALPHA_ENV_2
	case 0xc6: // TEV_COLOR_ENV_3
	case 0xc7: // TEV_ALPHA_ENV_3
	case 0xc8: // TEV_COLOR_ENV_4
	case 0xc9: // TEV_ALPHA_ENV_4
	case 0xca: // TEV_COLOR_ENV_5
	case 0xcb: // TEV_ALPHA_ENV_5
	case 0xcc: // TEV_COLOR_ENV_6
	case 0xcd: // TEV_ALPHA_ENV_6
	case 0xce: // TEV_COLOR_ENV_7
	case 0xcf: // TEV_ALPHA_ENV_7
	case 0xd0: // TEV_COLOR_ENV_8
	case 0xd1: // TEV_ALPHA_ENV_8
	case 0xd2: // TEV_COLOR_ENV_9
	case 0xd3: // TEV_ALPHA_ENV_9
	case 0xd4: // TEV_COLOR_ENV_A
	case 0xd5: // TEV_ALPHA_ENV_A
	case 0xd6: // TEV_COLOR_ENV_B
	case 0xd7: // TEV_ALPHA_ENV_B
	case 0xd8: // TEV_COLOR_ENV_C
	case 0xd9: // TEV_ALPHA_ENV_C
	case 0xda: // TEV_COLOR_ENV_D
	case 0xdb: // TEV_ALPHA_ENV_D
	case 0xdc: // TEV_COLOR_ENV_E
	case 0xdd: // TEV_ALPHA_ENV_E
	case 0xde: // TEV_COLOR_ENV_F
	case 0xdf: // TEV_ALPHA_ENV_F
		if(gx_tev::combiner[_addr - 0xc0] != _value)
		{
			gx_tev::combiner[_addr - 0xc0] = _value;
			gx_tev::set_modifed();
		}
		break;

	case 0xe0: // TEV_REGISTERL_0
	case 0xe2: // TEV_REGISTERL_1
	case 0xe4: // TEV_REGISTERL_2
	case 0xe6: // TEV_REGISTERL_3
	case 0xe1: // TEV_REGISTERH_0
	case 0xe3: // TEV_REGISTERH_1
	case 0xe5: // TEV_REGISTERH_2
	case 0xe7: // TEV_REGISTERH_3
		gx_tev::upload_color(_addr, _value);
		break;

	case 0xf3: // TEV_ALPHAFUNC
		gx_states::set_alphafunc();
		break;

	case 0xf6: // TEV_KSEL_0
	case 0xf7: // TEV_KSEL_1
	case 0xf8: // TEV_KSEL_2
	case 0xf9: // TEV_KSEL_3
	case 0xfa: // TEV_KSEL_4
	case 0xfb: // TEV_KSEL_5
	case 0xfc: // TEV_KSEL_6
	case 0xfd: // TEV_KSEL_7
		gx_tev::set_modifed();
		break;
	}
}

// Desc: Write data into a CP register
//

void GX_CPLoad(u8 _addr, u32 _value)
{
    //LOG_NOTICE(TGP, "CP_LOAD [%02x] = %08x", _addr, _value);
	// write data to cp memory
	cp.mem[_addr] = _value;
}

// Desc: Write data into a XF register
//

void GX_XFLoad(u16 _length, u16 _addr, u32 _regs[64])
{
	int i;

	// write data to xf memory/registers
	switch((_addr & XF_ADDR_MASK) >> 8)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03: // matrix transformation memory
	case 0x04: // normal transformation memory
	case 0x05: // dual texture transformation memory
	case 0x06: 
	case 0x07: // light transformation memory

		for(i = 0; i < _length; i++)
		{		
			((f32*)xfmem)[_addr + i] = toFLOAT(_regs[i]);
		}

		break;

	case 0x10: // registers

		u8 _maddr = (_addr & 0xff);

		for(i = 0; i < _length; i++)
		{
			xf.mem[_maddr + i] = _regs[i];
		}
		
		switch(_maddr)
		{
		case 0x1a:
		case 0x1b:
		case 0x1d:
		case 0x1e:
			gx_states::set_viewport();
			break;

		case 0x20:
			gx_states::set_projection();
			break;
		}

		break;
	}
}

// Desc: Write data into a XF register indexed-form
//

void GX_XFLoadIndexed(u8 _n, u16 _index, u8 _length, u16 _addr)
{
//	u32* ptr = ((u32*)MEMPTR8(CP_IDX_ADDR(_index, _n)));

	for(int i = 0; i < _length; i++)
	{
		xfmem[_addr + i] = Memory_Read32(CP_IDX_ADDR(_index, _n) + (i*4));// BSWAP32(ptr[i]);
	}
}

// Desc: Read/Write from/to GX Hardware
//

void EMU_FASTCALL GX_Fifo_Write8(u32 addr, u32 data)
{
    gp::g_fifo_buffer[gp::g_fifo_write_ptr++] = data;
}

void EMU_FASTCALL GX_Fifo_Write16(u32 addr, u32 data)
{
    *(u16*)(gp::g_fifo_buffer + gp::g_fifo_write_ptr) = BSWAP16(data);
    gp::g_fifo_write_ptr += 2;
}

void EMU_FASTCALL GX_Fifo_Write32(u32 addr, u32 data) {
    *(u32*)(gp::g_fifo_buffer + gp::g_fifo_write_ptr) = BSWAP32(data);
    gp::g_fifo_write_ptr += 4;
}


u16 EMU_FASTCALL GX_Fifo_Read16(u32 addr)
{
	return 0;
}

u32 EMU_FASTCALL GX_Fifo_Read32(u32 addr)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

// Desc: Open GX Hardware
//

void GX_Open(void)
{
	LOG_NOTICE(TGP, "initialized ok");

	// initialize GX
	gx_fifo::initialize();
	gx_vertex::initialize();
	gx_states::initialize();

#ifdef USE_NEW_VIDEO_CORE
    video_core::Init();
#endif

#ifdef USE_FIFO_RECORDING
    fifo_player::StartRecording("fifo_recording.gfp");
#endif

	// initialize OpenGL
	//OPENGL_Create(NULL);
}

////////////////////////////////////////////////////////////////////////////////
