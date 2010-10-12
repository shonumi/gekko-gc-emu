// hw_di.cpp
// (c) 2005,2006 Gekko Team

#include "../../emu.h"
#include "hw_di.h"
#include "hw_pi.h"
#include "../../dvd interface/realdvd.h"

sDI hw_di;

u8 *DVDDataBuff;

////////////////////////////////////////////////////////////
// DI - DVD Interface
// The DVD Interface is used for accessing GameCube disc media
// and transferring it into main memory.  The DVD Interface also
// decodes the GameCube's file structure system.
//
//			6/15/06		-	File Created. (Lightning)
//
//			Missing:
//						-	16bit Reads and Writes
////////////////////////////////////////////////////////////

void DIProcessCmd()
{
	u32	i;
	u32	ReadLen;
	u32	NewMemPtr;

	//process a command that was sent
	switch(hw_di.CmdBuff[0] >> 24)
	{
	case DI_CMD_INQUIRY:
		for(i = 0; i < 0x20; i+=4)
			Memory_Write32(hw_di.DMAMemory + i, 0);

//		memset(MEMPTR32(hw_di.DMAMemory), 0, 0x20);
		hw_di.DMALength = 0;
		printf(".Flipper: DVD Inquiry\n");
		break;

	case DI_CMD_READDATA:
		RealDVDSeek(REALDVD_LOWLEVEL, hw_di.CmdBuff[1] << 2, REALDVDSEEK_START);
//		hw_di.DMALength -= RealDVDRead(REALDVD_LOWLEVEL, MEMPTR32(hw_di.DMAMemory), hw_di.CmdBuff[2]);

		ReadLen = hw_di.CmdBuff[2];
		NewMemPtr = hw_di.DMAMemory;
		while(ReadLen >= 1024*1024)
		{
			hw_di.DMALength -= RealDVDRead(REALDVD_LOWLEVEL, (u32 *)DVDDataBuff, 1024*1024);

			for(i = 0; i < 1024*1024; i+=4)
				Memory_Write32(NewMemPtr + i, BSWAP32(*(u32 *)&DVDDataBuff[i]));

			NewMemPtr += 1024*1024;
			ReadLen -= 1024*1024;
		}

		hw_di.DMALength -= RealDVDRead(REALDVD_LOWLEVEL, (u32 *)DVDDataBuff, ReadLen);

		for(i = 0; i < (ReadLen >> 2); i++)
			Memory_Write32(NewMemPtr + (i * 4), BSWAP32(*(u32 *)&DVDDataBuff[(i * 4)]));

		for(i = (i * 4); i < ReadLen; i++)
			Memory_Write8(NewMemPtr + i, DVDDataBuff[i]);

//		hw_di.DMALength -= RealDVDRead(REALDVD_LOWLEVEL, MEMPTR32(hw_di.DMAMemory), hw_di.CmdBuff[2]);
//		cpu->CheckMemoryWrite(hw_di.DMAMemory, hw_di.CmdBuff[2]);
		//printf(".Flipper: DVD Read Loc %08X Len %08X to Mem %08X\n", hw_di.CmdBuff[1] << 2, hw_di.CmdBuff[2], hw_di.DMAMemory);

		/*if(bRescanOnDVDRead)
		{
			u32 start = hw_di.DMAMemory;
			u32 end = (hw_di.DMAMemory + hw_di.CmdBuff[2]);

			printf("Dynamically Linked Read - Rescanning %08x to %08x!\n", start, end);
			HLE_ScanRange(start, end);
			//bRescanOnDVDRead = false;
		}*/

		break;

	case DI_CMD_SEEK:
		break;

	case DI_CMD_REQUESTERR:
		break;

	case DI_CMD_PLAYAUDIO:
		break;

	case DI_CMD_REQAUDIOSTAT:
		break;

	case DI_CMD_STOPMOTOR:
		break;

	case DI_CMD_DVDAUDIO:
		switch((hw_di.CmdBuff[0] & 0x00FF0000) >> 16)
		{
			case DI_CMD_DVDAUDIO_DISABLE:
				break;

			case DI_CMD_DVDAUDIO_ENABLE:
				break;
		};

		default:
			printf(".Flipper: Undefined DI Command: %08X %08X %08X %08X %08X %08X %08X\n", \
						hw_di.CmdBuff[0], hw_di.CmdBuff[1], hw_di.CmdBuff[2], hw_di.DMAMemory, hw_di.DMALength, hw_di.IMMBuf, hw_di.cr);
	}

	//if the transfer interrupt is wanted, then assert it
	hw_di.sr |= DI_SR_TCINT;
	if(hw_di.sr & DI_SR_TCINTMASK)
		PI_RequestInterrupt(PI_MASK_DI);
}

////////////////////////////////////////////////////////////

// Desc: Read/Write from/to DI Hardware
//

u8  __fastcall DI_Read8(u32 addr)
{
	printf(".Flipper: Undefined DI_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall DI_Write8(u32 addr, u32 data)
{
	printf(".Flipper: Undefined DI_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall DI_Read16(u32 addr)
{
	printf(".Flipper: Undefined DI_Read16: %08X!\n", addr);
	return 0;
}

void  __fastcall DI_Write16(u32 addr, u32 data)
{
	printf(".Flipper: Undefined DI_Write16: %08X := %08X !\n", addr, data);
}

u32  __fastcall DI_Read32(u32 addr)
{
	switch(addr)
	{
		case DI_SR:
			return hw_di.sr;

		case DI_CVR:
			return hw_di.cvr;

		case DI_CMD0:
			return hw_di.CmdBuff[0];

		case DI_CMD1:
			return hw_di.CmdBuff[1];

		case DI_CMD2:
			return hw_di.CmdBuff[2];

		case DI_DMAMEM:
			return hw_di.DMAMemory;

		case DI_DMALEN:
			return hw_di.DMALength;

		case DI_CR:
			return hw_di.cr;

		case DI_IMMBUF:
			return hw_di.IMMBuf;

		case DI_CFG:
			return hw_di.cfg;

		default:
			printf(".Flipper: Undefined DI_Read32: %08X!\n", addr);
			return 0;
	}
}

void  __fastcall DI_Write32(u32 addr, u32 data)
{
	switch(addr)
	{
		case DI_SR:
			//set the mask flags as needed
			if(data & DI_SR_BRKINTMASK)
				hw_di.sr |= DI_SR_BRKINTMASK;
			else
				hw_di.sr &= ~DI_SR_BRKINTMASK;

			if(data & DI_SR_DEINTMASK)
				hw_di.sr |= DI_SR_DEINTMASK;
			else
				hw_di.sr &= ~DI_SR_DEINTMASK;

			if(data & DI_SR_TCINTMASK)
				hw_di.sr |= DI_SR_TCINTMASK;
			else
				hw_di.sr &= ~DI_SR_TCINTMASK;

			//clear the interrupts
			if(data & DI_SR_BRKINT)
			{
				hw_di.sr &= ~DI_SR_BRKINT;
				PI_ClearInterrupt(PI_MASK_DI);
			}

			if(data & DI_SR_DEINT)
			{
				hw_di.sr &= ~DI_SR_DEINT;
				PI_ClearInterrupt(PI_MASK_DI);
			}

			if(data & DI_SR_TCINT)
			{
				hw_di.sr &= ~DI_SR_TCINT;
				PI_ClearInterrupt(PI_MASK_DI);
			}

			//if the brk is set then handle it
			if(data & DI_SR_BRK)
			{
				hw_di.sr &= ~DI_SR_BRK;
				hw_di.sr |= DI_SR_BRKINT;
				if(hw_di.sr & DI_SR_BRKINTMASK)
					PI_RequestInterrupt(PI_MASK_DI);
			}
			return;

		case DI_CVR:
			//handle the interrupt for the cover
			if(data & DI_CVR_CVRINT)
			{
				//clear it
				hw_di.cvr &= ~DI_CVR_CVRINT;
				PI_ClearInterrupt(PI_MASK_DI);
			}

			//handle the mask
			if(data & DI_CVR_CVRINTMASK)
				hw_di.cvr |= DI_CVR_CVRINTMASK;
			else
				hw_di.cvr &= ~DI_CVR_CVRINTMASK;

			return;

		case DI_CMD0:
			hw_di.CmdBuff[0] = data;
			return;

		case DI_CMD1:
			hw_di.CmdBuff[1] = data;
			return;

		case DI_CMD2:
			hw_di.CmdBuff[2] = data;
			return;

		case DI_DMAMEM:
			hw_di.DMAMemory = data;
			return;

		case DI_DMALEN:
			hw_di.DMALength = data;
			return;

		case DI_CR:
			hw_di.cr = data;


			//if the start flag then process the command
			if(hw_di.cr & DI_CR_TSTART)
				DIProcessCmd();

			return;

		case DI_IMMBUF:
			hw_di.IMMBuf = data;
			return;

		case DI_CFG:
			//read only
			return;

		default:
			printf(".Flipper: Undefined DI_Write32: %08X := %08X !\n", addr, data);
			return;
	}
}

// Desc: Initialize DI Hardware
//

void DI_Open(void)
{
	printf("DI_Open()\n");

	memset(&hw_di, 0, sizeof(hw_di));

	DVDDataBuff = (u8*)malloc(1024*1024);
}

void DI_Close(void)
{
	free(DVDDataBuff);
	DVDDataBuff = 0;
}

////////////////////////////////////////////////////////////
