// hw_dsp.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "powerpc/cpu_core.h"
#include "hw.h"
#include "hw_dsp.h"
#include "hw_pi.h"
#include "hw_ai.h"
#include "hle/hle_dsp.h"
#include "powerpc/cpu_core_regs.h"

//TODO: Code cleanup (shonumi) and soon too

sDSP	dsp;
u8		DSPRegisters[REG_SIZE];
u8		ARAM[ARAM_SIZE];
s64		g_DSPDMATime;
u32		dspDMALenENBSet = 0;
u32		dspCSRDSPIntMask = 0;
u32		dspCSRDSPInt = 0;

DSPHLE dsp_emulator;

u16		g_AR_INFO;
u16		g_AR_MODE;
u16		g_AR_REFRESH;

////////////////////////////////////////////////////////////
// DSP - Digital Signal Processor
// Currently, the Gamecube's custom DSP audio hardware is
// unknown.  Duddie has made an incomplete reverse GCEmu, but
// that is not entirelly correct at all.  This file serves only
// to handle basic dsp "mail" registers, so that most programs
// can successfully pass SDK initialization.
//
//			1/15/06		-	File Created. (ShizZy)
//			3/25/06		-	Basic DSP implementation - simple
//							mailbox register handling. (ShizZy)
//			7/20/06		-	Added support for ARAM DMA registers,
//							as well as preliminary interrupt handling.
//			8/3/06		-	Added extended support for ARAM DMA.
//
//			Missing:
//						-	Additional DMA Support
//						-	Audio Stream Playing
//						-	Volume Adjusting
////////////////////////////////////////////////////////////

// Desc: Get DMA Time in ticks.
//

s64 DSP_GetDMATime(long _len, long _rate)
{
    long samples = _len / 4;
    return samples * (cpu->GetTicksPerSecond() / _rate);
}

// Desc: Audio RAM Hardware Interrupt Control
//

void AudioRam_Interrupt(void)
{
    REGDSP16(DSP_CSR) |= DSP_CSR_ARINT;
    if(REGDSP16(DSP_CSR) & DSP_CSR_ARINTMSK)
    {
        PI_RequestInterrupt(PI_MASK_DSP);
    }
}

// Desc: Initiate a DMA to and from Audio RAM
//

void AudioRam_DMA(u32 _type, u32 _maddr, u32 _aaddr, u32 _size)
{
	u32		i;

	//printf("AudioRam_DMA\n");
	if(dsp.cntv[0] && dsp.cntv[1])												// If Enabled?
    {
        dsp.cntv[0] = dsp.cntv[1] = false;										// Disable

		if(_aaddr < ARAM_SIZE)													// Don't Overflow...
		{
			//avoid the shift of the data and just compare it
			if(_type & 0x80000000)
			{
				//ARAM to RAM
				for(i = 0; i < (_size >> 2); i++)
					Memory_Write32(_maddr + (i * 4), BSWAP32(*(u32 *)&ARAM[_aaddr + (i * 4)]));

				for(i = i * 4; i < _size; i++)
					Memory_Write8(_maddr + i, *(u32 *)&ARAM[_aaddr + i]);
			}
			else
			{
				//RAM to ARAM
				for(i = 0; i < (_size >> 2); i++)
					*(u32 *)&ARAM[_aaddr + (i * 4)] = BSWAP32(Memory_Read32(_maddr + (i * 4)));

				for(i = i * 4; i < _size; i++)
					ARAM[_aaddr + i] = Memory_Read8(_maddr + i);
			}

			REGDSP32(DSP_AR_DMA_CNT) &= 0x80000000;								// Reset count register
			AudioRam_Interrupt();												// Interrupt
		}
		else
		{
			//trying to access too large, reset ram if need be
			if(!_type)
			{
				for(i = 0; i < (_size >> 2); i++)
					Memory_Write32(_maddr + (i * 4), 0);

				for(i = i * 4; i < _size; i++)
					Memory_Write8(_maddr + i, 0);

				REGDSP32(DSP_AR_DMA_CNT) &= 0x80000000;								// Reset count register
				AudioRam_Interrupt();												// Interrupt
			}
		}
    }
}

////////////////////////////////////////////////////////////

// Desc: Read/Write from/to DSP Hardware
//

u8 EMU_FASTCALL DSP_Read8(u32 addr)
{
	printf("~Flipper: Undefined DSP_Read8: %08X!\n", addr);
	return 0;
}

void EMU_FASTCALL DSP_Write8(u32 addr, u32 data)
{
	printf("~Flipper: Undefined DSP_Write8: %08X := %08X !\n", addr, data);
}

u16 EMU_FASTCALL DSP_Read16(u32 addr)
{
	switch(addr)
	{
	case CPU_DSP_MAILBOX_HI:
		//printf("CPU checks DSP mbox PC=%08x (%04x)\n",ireg_PC(),REGDSP16(CPU_DSP_MAILBOX_HI));
                return dsp_emulator.DSP_ReadMailboxHi(true);

	case CPU_DSP_MAILBOX_LO:
		//printf("CPU checks DSP mbox+2 PC=%08x\n",ireg_PC());
                return dsp_emulator.DSP_ReadMailboxLo(true);

	case DSP_CPU_MAILBOX_HI:
                //printf("CPU (%08x) checks mbox, ",ireg_PC());
                return dsp_emulator.DSP_ReadMailboxHi(false);

	case DSP_CPU_MAILBOX_LO:
                //printf("CPU (%08x) checks mbox+2, gets %04x\n",ireg_PC(),REGDSP16(DSP_CPU_MAILBOX_LO));
                return dsp_emulator.DSP_ReadMailboxLo(false);

	case DSP_CSR:
		//printf("reading DSP_CSR=%04x\n",REGDSP16(DSP_CSR));
		return REGDSP16(DSP_CSR);

	case DSP_AR_DMA_MMADDR:
	case DSP_AR_DMA_MMADDR + 2:
	case DSP_AR_DMA_ARADDR:
	case DSP_AR_DMA_ARADDR + 2:
	case DSP_AR_DMA_CNT:
	case DSP_AR_DMA_CNT + 2:
		return REGDSP16(addr);

	case DSP_AR_INFO:
		return g_AR_INFO;

	case DSP_AR_MODE:
		return g_AR_MODE;

	case DSP_AR_REFRESH:
		return g_AR_REFRESH;

	case DSP_DMA_ADDR:
		//printf("DSP_DMA_ADDR read %04x\n",REGDSP16(DSP_DMA_ADDR));
		return REGDSP16(DSP_DMA_ADDR);
	case DSP_DMA_ADDR+2:
		//printf("DSP_DMA_ADDR+2 read %04x\n",REGDSP16(DSP_DMA_ADDR+2));
		return REGDSP16(DSP_DMA_ADDR+2);

	case DSP_DMA_CNT:
		REGDSP16(DSP_DMA_CNT)--;
		return REGDSP16(DSP_DMA_CNT);

	case DSP_DMA_LEN:
		return REGDSP16(DSP_DMA_LEN);

	default:
		printf("~Flipper: Undefined DSP_Read16: %08X!\n", addr);
		return 0;
	}
}

void EMU_FASTCALL DSP_Write16(u32 addr, u32 data)
{
	switch(addr)
	{
	case CPU_DSP_MAILBOX_HI:
		printf("CPU writes CPU_DSP_MAILBOX_HI %04x\n",data&0xffff);
                dsp_emulator.DSP_WriteMailboxHi(true, data);
		return;

	case CPU_DSP_MAILBOX_LO:
		printf("CPU writes CPU_DSP_MAILBOX_LO %04x\n",data&0xffff);
                dsp_emulator.DSP_WriteMailboxLo(true, data);
		return;

	case DSP_CPU_MAILBOX_HI:
	case DSP_CPU_MAILBOX_LO:
		return;
	case DSP_DMA_CNT:
		return;

	case DSP_CSR:
		REGDSP16(DSP_CSR) = data;
		dspCSRDSPIntMask = data & DSP_CSR_DSPINTMSK;

		if (REGDSP16(DSP_CSR) & DSP_CSR_DSPINT)
		{
			REGDSP16(DSP_CSR)  &= ~DSP_CSR_DSPINT;
			dspCSRDSPInt = 0;
			PI_ClearInterrupt(PI_MASK_DSP);
		}
		if (REGDSP16(DSP_CSR) & DSP_CSR_ARINT)
		{
			REGDSP16(DSP_CSR)  &= ~DSP_CSR_ARINT;
			PI_ClearInterrupt(PI_MASK_DSP);
		}
		if (REGDSP16(DSP_CSR) & DSP_CSR_AIDINT)
		{
			REGDSP16(DSP_CSR)  &= ~DSP_CSR_AIDINT;
			PI_ClearInterrupt(PI_MASK_DSP);
		}

		REGDSP16(DSP_CSR) &= ~DSP_CSR_DMAINT;		// hack

		return;

	case DSP_AR_DMA_MMADDR:
	case DSP_AR_DMA_MMADDR + 2:
	case DSP_AR_DMA_ARADDR:
	case DSP_AR_DMA_ARADDR + 2:
		REGDSP16(addr) = data;
		return;

	case DSP_AR_INFO:
		g_AR_INFO = data;

	case DSP_AR_MODE:
		g_AR_MODE = data;
		return;

	case DSP_AR_REFRESH:
		g_AR_REFRESH = data;
		return;
		
	case DSP_AR_DMA_CNT:
		dsp.cntv[0] = true;
		REGDSP16(addr) = data;
		AudioRam_DMA(REGDSP32(DSP_AR_DMA_CNT), REGDSP32(DSP_AR_DMA_MMADDR), REGDSP32(DSP_AR_DMA_ARADDR), ARAM_DMA_SIZE);
		return;

	case DSP_AR_DMA_CNT + 2:
		dsp.cntv[1] = true;
		REGDSP16(addr) = data;
		AudioRam_DMA(REGDSP32(DSP_AR_DMA_CNT), REGDSP32(DSP_AR_DMA_MMADDR), REGDSP32(DSP_AR_DMA_ARADDR), ARAM_DMA_SIZE);
		return;

	case DSP_DMA_ADDR:
		//printf("DSP_DMA_ADDR write %04x\n",data);
		REGDSP16(DSP_DMA_ADDR) = data;
		return;
	case DSP_DMA_ADDR+2:
		//printf("DSP_DMA_ADDR+2 write %04x\n",data);
		REGDSP16(DSP_DMA_ADDR+2) = data;
		return;

	case DSP_DMA_LEN:
		REGDSP16(DSP_DMA_LEN) = data;
		dspDMALenENBSet = (data & DSP_DMALEN_ENB);
		if(data & DSP_DMALEN_ENB)
		{
			//printf("AI DMA from RAM len = %08x? %08x\n",data,REGDSP32(DSP_DMA_ADDR));
			//REGDSP16(DSP_DMA_CNT) = REGDSP16(DSP_DMA_LEN) & ~DSP_DMALEN_ENB;

			/* do the finished interrupt now */
			//REGDSP16(DSP_CSR)  |= ~DSP_CSR_AIDINT;
			//PI_RequestInterrupt(PI_MASK_DSP);
			//PI_RequestInterrupt(PI_MASK_DSP);
		} else {
			//printf("Stop sample?\n");
			//PI_ClearInterrupt(PI_MASK_DSP);
		}
		break;

	default:
		printf("~Flipper: Undefined DSP_Write16: %08X := %08X !\n", addr, data);
		return;
	}
}

u32 EMU_FASTCALL DSP_Read32(u32 addr)
{
	LOG_ERROR(TDSP, "~Flipper: DSP_Read32: %08X (PC=%08x)!\n", addr, ireg.PC);
	switch(addr)
	{
	case DSP_AR_DMA_MMADDR:
	case DSP_AR_DMA_ARADDR:
	case DSP_AR_DMA_CNT:
		return REGDSP32(addr);

	default:
		printf("~Flipper: Undefined DSP_Read32: %08X!\n", addr);
		return 0;
	}
}

void EMU_FASTCALL DSP_Write32(u32 addr, u32 data)
{
	//DisplayError("~Flipper: DSP_Write32: %08X (PC=%08x)!\n", addr, ireg_PC());
	
	switch(addr)
	{
	case DSP_AR_DMA_MMADDR:
	case DSP_AR_DMA_ARADDR:
		REGDSP32(addr) = data;
		return;

	case DSP_AR_DMA_CNT:
		dsp.cntv[0] = dsp.cntv[1] = true;
		REGDSP32(DSP_AR_DMA_CNT) = data;
		AudioRam_DMA(ARAM_DMA_TYPE, REGDSP32(DSP_AR_DMA_MMADDR), REGDSP32(DSP_AR_DMA_ARADDR), ARAM_DMA_SIZE);
		return;

	default:
		printf("~Flipper: Undefined DSP_Write32: %08X := %08X !\n", addr, data);
		return;
	}
}

////////////////////////////////////////////////////////////

// Desc: Update DSP Hardware
//

void DSP_Update(void)
{
	// DSP DMA (interrupt)

	if(!dspDMALenENBSet || (ireg.TBR.TBR < (u64)g_DSPDMATime))
	{}
	else
	{
		REGDSP16(DSP_DMA_CNT) = REGDSP16(DSP_DMA_LEN) & ~DSP_DMALEN_ENB;
		g_DSPDMATime = cpu->GetTicks() + DSP_GetDMATime(REGDSP16(DSP_DMA_CNT) * 32, g_AISampleRate);

		REGDSP16(DSP_CSR) |= DSP_CSR_AIDINT;
		if(REGDSP16(DSP_CSR) & DSP_CSR_AIDINTMSK)
		{
			PI_RequestInterrupt(PI_MASK_DSP);
		}
	}

	if (!dspCSRDSPInt || !dspCSRDSPIntMask)
		return;
	else
	{
		PI_RequestInterrupt(PI_MASK_DSP);
	}
}

// Desc: Initialize DSP Hardware
//

void DSP_Open(void)
{
	LOG_NOTICE(TDSP, "initialized ok");

	memset(&dsp, 0, sizeof(sDSP));
	memset(DSPRegisters, 0, sizeof(DSPRegisters));
	memset(ARAM, 0, sizeof(ARAM));

	dsp_emulator.SetUCode(UCODE_ROM);

	g_DSPDMATime = 0;
	g_AISampleRate = 32000;
	g_AR_INFO = 0;
	g_AR_MODE = 1;
        g_AR_REFRESH = 156;
}

////////////////////////////////////////////////////////////
