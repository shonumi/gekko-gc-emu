// hw_exi.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "memory.h"
#include "hw.h"
#include "hw_exi.h"
#include "hw_pi.h"

#include <time.h>
#include <sys/timeb.h>

//

sEXI	exi;
u8		*IPLRom = 0;
u8		*SRAM = 0;

u64		EXIMask = ((u64)(EXI_CSR_EXIINTMASK | EXI_CSR_EXTINTMASK | EXI_CSR_TCINTMASK) << 32) |
				  (EXI_CSR_EXIINTMASK | EXI_CSR_EXTINTMASK | EXI_CSR_TCINTMASK);

typedef void(*EXIFunc)(u32 addr);

////////////////////////////////////////////////////////////
// EXI - External Interface
// The external interface is used for DMA access to external 
// memory.  Such includes memory cards, as well as internal
// Gamecube ROM (Fonts, etc).  Currently, no DMA is implemented,
// but the registers are covered to the extent that they should
// not cause any simple roms to hang.
//
//			3/22/08		-	Deleted original code, started fresh. Too much missing
////////////////////////////////////////////////////////////

#include "data/font_ansi.h"
#include "data/font_sjis.h" // (compiler limit: divided into 3 arrays)

////////////////////////////////////////////////////////////

// Desc: Transfer from MX Hardware (Only DMA Supported)
//

void MX_Transfer(u32 addr)
{
	u32		offset;
	time_t	CurTime;
	u32		i;

	switch((exi.cr[0] & EXI_CR_RW) >> 2)
	{
		case 0:											// MX Read
			if(exi.cr[0] & EXI_CR_DMA)
			{
				offset = exi.data[0] & 0x7FFFFFFF;

				if(offset == 0x20000000)
				{
					printf(".EXI DMA access to RTC not supported!\n");
				}
				if(offset == 0x20000100)
				{
					for(i = 0; i < 64; i+=4)
						Memory_Write32(exi.mar[0] + i, BSWAP32(*(u32 *)&SRAM[i]));
//					memcpy(&RAM[exi.mar[0] & RAM_MASK], &SRAM[0], 64);
				}
				else if((offset >= 0x00000000) && (offset < 0x08000000))
				{
					for(i = 0; i < (exi.len[0] >> 2); i++)
						Memory_Write32(exi.mar[0] + (i * 4), BSWAP32(*(u32 *)&IPLRom[(offset >> 6) + (i * 4)]));

					for(i = (i*4); i < exi.len[0]; i++)
						Memory_Write8(exi.mar[0] + i, *(u32 *)&IPLRom[(offset >> 6) + i]);

//					memcpy(&RAM[exi.mar[0] & RAM_MASK], &IPLRom[offset >> 6], exi.len[0]);
				}
				else
					printf(".EXI Undefined MX Read!\n");
			}
			else
			{
				if(exi.data[0] == 0x20000000)
				{
					//set the RTC time
					time(&CurTime);
					exi.data[0] = (u32)CurTime - (60*60*24*365);
				}
				else
					printf(".EXI Undefined MX Read %08X!\n", exi.data[0]);
			}
			break;
		case 1:											// MX Write
			printf(".EXI Undefined MX Write %08X!\n", exi.data[0]);
			break;
		case 2:											// MX Read and write, invalid for DMA.
			printf(".EXI Undefined MX Read/Write %08X!\n", exi.data[0]);
			break;
		case 3:											// Undefined
			printf(".EXI Undefined MX Tranfer Type!\n");
			break;
	}
}


void EXI_UnknownTransfer(u32 addr)
{
	u32 Chan;
	u32 Device;

	switch(addr)
	{
		case EXI_CR0:					// Control Register
			Chan = 0;
			break;

		case EXI_CR1:					// ...
			Chan = 1;
			break;

		case EXI_CR2:					// ...
			Chan = 2;
			break;
	}

	switch((exi.csr[Chan] >> 7) & 7)
	{
		case 1:
			Device = 0;
			break;

		case 2:
			Device = 1;
			break;

		case 4:
			Device = 2;
			break;

		case 0:
		case 3:
		case 5:
		case 6:
		case 7:
			Device = -((exi.csr[Chan] >> 7) & 7);	//invalid device ID
	};

	if(exi.cr[Chan] & EXI_CR_DMA)
		printf(".EXI Unknown DMA transfer: Chan %d Device %d Len %08X Data %08X\n", Chan, Device, exi.len[Chan]);
	else
		printf(".EXI Unknown IMM transfer: Chan %d Device %d Data %08X CR %08X\n", Chan, Device, exi.data[Chan], exi.cr[Chan]);
}

void AD16_Transfer(u32 addr)
{
	printf(".EXI AD16 Access Not Implemented!\n");
}

void SerialEthernet_Transfer(u32 addr)
{
	printf(".EXI Serial/Ethernet Access Not Implemented!\n");
}

//only 1 of each bit should be set. this is a catch all incase there is invalid transfers
//as a result, Device 0 = Entry 1, Device 1 = Entry 2, and Device 2 = Entry 4 
EXIFunc EXI_Transfer[8*3] = 
{
	EXI_UnknownTransfer, MemCard_Transfer, MX_Transfer, EXI_UnknownTransfer, SerialEthernet_Transfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer,
	EXI_UnknownTransfer, MemCard_Transfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer,
	EXI_UnknownTransfer, AD16_Transfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer, EXI_UnknownTransfer
};

////////////////////////////////////////////////////////////

// Desc: Read/Write from/to EXI Hardware
//

u8 __fastcall EXI_Read8(u32 addr)
{
	printf("~Flipper: Undefined EXI_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall EXI_Write8(u32 addr, u32 data)
{
	printf("~Flipper: Undefined EXI_Write8: %08X := %08X !\n", addr, data);
	return;
}

u16 __fastcall EXI_Read16(u32 addr)
{
	printf("~Flipper: Undefined EXI_Read16: %08X!\n", addr);
	return 0;
}

void __fastcall EXI_Write16(u32 addr, u32 data)
{
	printf("~Flipper: Undefined EXI_Write16: %08X := %08X !\n", addr, data);
	return;
}

u32 __fastcall EXI_Read32(u32 addr)
{
	//printf(".EXI Read32: addr %08X\n", addr);
	switch(addr)
	{
	case EXI_CSR0:					// Channel Status Register
		return exi.csr[0];
	case EXI_CSR1:					// ...
		return exi.csr[1];
	case EXI_CSR2:					// ...
		return exi.csr[2];
	case EXI_CR0:					// Control Register
		return exi.cr[0];
	case EXI_CR1:					// ...
		return exi.cr[1];
	case EXI_CR2:					// ...
		return exi.cr[2];
	case EXI_MAR0:					// DMA Start Address
		return exi.mar[0];
	case EXI_MAR1:					// ...
		return exi.mar[1];
	case EXI_MAR2:					// ...
		return exi.mar[2];
	case EXI_LENGTH0:				// DMA Length
		return exi.len[0];
	case EXI_LENGTH1:				// ...
		return exi.len[1];
	case EXI_LENGTH2:				// ...
		return exi.len[2];
	case EXI_DATA0:					// Immediate Data
		return exi.data[0];
	case EXI_DATA1:					// ...
		return exi.data[1];
	case EXI_DATA2:					// ...
		return exi.data[2];

	default:
		printf("~Flipper: Undefined EXI_Read32: %08X!\n", addr);
		return 0;
	}
}

void __fastcall EXI_Write32(u32 addr, u32 data)
{
	u32		CRVal=0;
	u32		Device;
	u32		x;

	//printf(".EXI Write32: addr %08X data %08X\n", addr, data);
	switch(addr)
	{
	case EXI_CSR2:
		CRVal++;
	case EXI_CSR1:
		CRVal++;
	case EXI_CSR0:					// Channel Status Register
		//make sure no interrupts are set that they wish to clear
		exi.csr[CRVal] &= ~(data & EXI_CSR_EXIINT | EXI_CSR_EXTINT | EXI_CSR_TCINT);

		//update, make sure the interrupts are not set in the data
		exi.csr[CRVal] |= (data & ~(EXI_CSR_EXIINT | EXI_CSR_EXTINT | EXI_CSR_TCINT | EXI_CSR_EXT));

		//get the interrupt masks for the active interrupts
		exi.csrINTMask[CRVal] = (exi.csr[CRVal] & (EXI_CSR_EXIINTMASK | EXI_CSR_EXTINTMASK | EXI_CSR_TCINTMASK)) << 1;
/*
		//now apply the mask to anything that may be active
		exi.csrINTMask[CRVal] &= exi.csr[CRVal];

		//generate the final mask
		exi.FinalINTMask = exi.csrINTMask[0] |
						   exi.csrINTMask[1] |
						   exi.csrINTMask[2];
*/
//		printf(".EXI: EXI CSR Write Chan %d Dev 0x%02X  %08X %08X\n", CRVal, ((exi.csr[CRVal] >> 7) & 7), exi.csr[CRVal], exi.data[CRVal]);
		EXI_Update();
		return;

	case EXI_CR2:					// ...
		CRVal++;
	case EXI_CR1:					// ...
		CRVal++;
	case EXI_CR0:					// Control Register
		exi.cr[CRVal] = data;
		if(exi.cr[CRVal] & EXI_CR_TSTART)
		{
			for(x = 0; x < 3; x++)
			{
				Device = (exi.csr[CRVal] >> 7) & (1 << x);
				if(Device)
					EXI_Transfer[(CRVal*8) + Device](addr);
			}

			exi.cr[CRVal] &= ~EXI_CR_TSTART;				// Complete Transfer Start
			exi.csr[CRVal] |= EXI_CSR_TCINT;			// Enable CSR Interrupt

			EXI_Update();
		}
		return;

	case EXI_MAR0:					// DMA Start Address
		exi.mar[0] = data;
		return;
	case EXI_MAR1:					// ...
		exi.mar[1] = data;
		return;
	case EXI_MAR2:					// ...
		exi.mar[2] = data;
		return;
	case EXI_LENGTH0:				// DMA Length
		exi.len[0] = data;
		return;
	case EXI_LENGTH1:				// ...
		exi.len[1] = data;
		return;
	case EXI_LENGTH2:				// ...
		exi.len[2] = data;
		return;
	case EXI_DATA0:					// Immediate Data
		exi.data[0] = data;
		return;
	case EXI_DATA1:					// ...
		exi.data[1] = data;
		return;
	case EXI_DATA2:					// ...
		exi.data[2] = data;
		return;

	default:
		printf("~Flipper: Undefined EXI_Write32: %08X := %08X !\n", addr, data);
		return;
	}
}

////////////////////////////////////////////////////////////

// Desc: Update EXI Hardware
//

void EXI_Update(void)
{
	static int LastINTStatus = -1;
/*
	if(!MemCardBusy[2])
	{}
	else
		MemCard_Update();
*/

#ifndef USE_INLINE_ASM
	//generate the final mask
	exi.FinalINTMask = (exi.csrINTMask[0] & exi.csr[0]) |
						(exi.csrINTMask[1] & exi.csr[1]) |
						(exi.csrINTMask[2] & exi.csr[2]);

	//if we have an interrupt then set it
	if(exi.FinalINTMask)
	{
		if(!LastINTStatus) // != 1)
		{
			PI_RequestInterrupt(PI_MASK_EXI);
			LastINTStatus = 1;
		}
	}
	else
	{
		if(LastINTStatus) // != 0)
		{
			LastINTStatus = 0;
			PI_ClearInterrupt(PI_MASK_EXI);
		}
	}
#else
	_asm
	{
		movdqu xmm0, xmmword ptr exi.csr
		movdqu xmm1, xmmword ptr exi.csrINTMask
		pxor xmm2, xmm2
		pand xmm0, xmm1
		pcmpgtd xmm0, xmm2
		movmskps eax, xmm0
		mov ecx, PI_MASK_EXI

		//see if we have any interrupts
		cmp eax, 0
		jne EXI_Interrupt

		//no interrupt, if we haven't already cleared the interrupt
		//then  clear it
		cmp LastINTStatus, 0
		je EXI_InterruptDone

		mov LastINTStatus, 0
		call PI_ClearInterrupt
		jmp EXI_InterruptDone

EXI_Interrupt:
		//if we haven't already ran the interrupt then run it
		cmp LastINTStatus, 0
		jne EXI_InterruptDone

		mov LastINTStatus, 1
		call PI_RequestInterrupt

EXI_InterruptDone:
	};
#endif
}

// Desc: Initialize EXI Hardware
//

void EXI_Open(void)
{
	u32		x;
	timeb	LocalTime;
	time_t	CurTime;

	char IPLCopyMsg[] = "(C) 1999-2001 Nintendo. All rights reserved.(C) 1999 ArtX Inc. All rights reserved.PAL Revision 1.0";

	//setup the IPL Rom, based on layout from YAGCD
	IPLRom = (u8 *)malloc(0x200000);
	memset(&IPLRom[0], 0, 0x200000);

	//see if we are in pal mode
	if(Memory_Read8(0x80000003) == (u8)'P')
		memcpy(&IPLRom[0], IPLCopyMsg, sizeof(IPLCopyMsg));
	else
		memset(&IPLRom[0], 0, sizeof(IPLCopyMsg));
	memcpy(&IPLRom[0x001AFF00], font_sjis_a, sizeof(font_sjis_a));
	memcpy(&IPLRom[0x001AFF00 + sizeof(font_sjis_a)], font_sjis_b, sizeof(font_sjis_b));
	memcpy(&IPLRom[0x001AFF00 + sizeof(font_sjis_a) + sizeof(font_sjis_b)], font_sjis_c, sizeof(font_sjis_c));
	memcpy(&IPLRom[0x001FCF00], font_ansi, sizeof(font_ansi));
	
	if(Memory_Read8(0x80000003) == (u8)'P')
	{
		memset(&IPLRom[0x001FFF00], 0xFF, 0xFF);
		for(x = 0; x < 0x61; x++)
		{
			IPLRom[0x001FA0E0 + (x * 2)] = 0xFF;
			IPLRom[0x001FA0E0 + (x * 2) + 1] = 0x62;
		}
	}

	SRAM = (u8 *)malloc(64);
	memset(SRAM, 0, 64);
	SRAM[0x13] = 0x2C;		//stereo flag = 0x04

	//if the flash ID and CRC do not match/are missing
	//then memory cards will fail to mount
	//We just set the Flash ID to all 0's and create a CRC

	//generate the CRCs
	for(x = 0x04; x < 0x14; x+=2)
		*(u16 *)&SRAM[0] += *(u16 *)&SRAM[x];
	*(u16 *)&SRAM[2] = *(u16 *)&SRAM[0] ^ 0xFFFF;

	//flash CRCs
	for(x = 0x14; x < 0x20; x++)
	{
		SRAM[0x3A] += SRAM[x];
		SRAM[0x3B] += SRAM[x + 12];
	}
	SRAM[0x3A] = ~SRAM[0x3A];
	SRAM[0x3B] = ~SRAM[0x3B];

	_tzset();
	time(&CurTime);
	ftime(&LocalTime);

	//set the timezone offset
	*(u32 *)&SRAM[0x0C] = BSWAP32((u32)LocalTime.timezone * 60 * 60);

	memset(&exi, 0, sizeof(sEXI));

	exi.csr[0] = EXI_CSR_EXT | EXI_CSR_EXTINT;
	exi.csr[1] = exi.csr[0];

	MemCard_Open();

	LOG_NOTICE(TEXI, "initialized ok");
}

void EXI_Close(void)
{
	MemCard_Close();

	if(IPLRom)
		free(IPLRom);
	if(SRAM)
		free(SRAM);
}

////////////////////////////////////////////////////////////
