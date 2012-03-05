#include "common.h"
#include "memory.h"
#include "hw_exi.h"

u32	WriteBuff[4];
u32	WriteBuffPtr = 0;
u32	WriteBlockCount = 0;

u8	*MemCardData[2];
#define MemCardBlocks 64
#define MemCardSize	(0x2000 * MemCardBlocks)
#define MemCardSizeMask	0x0FFFFF
#define MemBlockType 0		//0 - ascii, 1 - japan (not working yet)

u32	MemCardStatus[2];
u32 MemCardInterruptSet[2];
u32 MemCardErasing[2];
u32 MemCardBusy[3] = {0, 0, 0};

#define MCSTATUS_BUSY		0x80000000
#define MCSTATUS_UNLOCKED	0x40000000
#define MCSTATUS_READY		0x01000000

u32 MemCard_ConvertOffset(u32 Pos1, u32 Pos2)
{
	u32	Position;

	//need the lower 2 bytes of Pos1 and the top 2 bytes of Pos2
	Position = ((Pos1 & 0xFFFFFF) << 8) | ((Pos2 & 0xFF000000) >> 24);

	//convert our offset and read the data
	return  ((Position & 0x3FFF0000) >> 7) |
			((Position & 0x00000300) >> 1) |
			(Position & 0x0000007F);
}

void MemCard_Transfer(u32 addr)
{
	u32			Channel;
	u32			Offset;
	u32			i;

	if(addr == EXI_CR0)
		Channel = 0;
	else
		Channel = 1;

	if(exi.cr[Channel] & EXI_CR_DMA)
	{
		//DMA access
		switch((exi.cr[Channel] & EXI_CR_RW) >> 2)
		{
		case 0:		//read
			Offset = MemCard_ConvertOffset(WriteBuff[0], WriteBuff[1]);
			printf(".EXI: DMA Memory Card %c Read %04X bytes from MC %08X to RAM %08X\n", 'A' + Channel, exi.len[Channel], Offset, exi.mar[Channel]);
			for(i = 0; i < (exi.len[Channel] >> 2); i++)
				Memory_Write32((exi.mar[Channel] + (i * 4)), BSWAP32((*(u32 *)(&MemCardData[Channel][(Offset + (i * 4)) & MemCardSizeMask]))));

			for(i = (i*4); i < exi.len[Channel]; i++)
				Memory_Write8((exi.mar[Channel] + i), (*(u32 *)(&MemCardData[Channel][(Offset + i) & MemCardSizeMask])));

			MemCardInterruptSet[Channel] = 1;
			break;

		case 1:		//write
			Offset = MemCard_ConvertOffset(WriteBuff[0], WriteBuff[1]);
//			printf(".EXI: DMA Memory Card %c Wrote %04X bytes from RAM %08X to MC %08X\n", 'A' + Channel, exi.len[Channel], exi.mar[Channel], Offset);

			for(i = 0; i < (exi.len[Channel] >> 2); i++)
			{
				*(u32 *)&MemCardData[Channel][(Offset + (i * 4)) & MemCardSizeMask] =
						BSWAP32(Memory_Read32((exi.mar[Channel] + (i * 4))));
			}

			for(i = (i*4); i < exi.len[Channel]; i++)
			{
				MemCardData[Channel][(Offset + i) & MemCardSizeMask] =
					Memory_Read8(exi.mar[Channel] + i);
			}

//			memcpy(&MemCardData[Channel][Offset & MemCardSizeMask], &RAM[exi.mar[Channel] & RAM_MASK], exi.len[Channel]);
			MemCardInterruptSet[Channel] = 1;
			break;

		case 2:		//read/write
			printf(".EXI: Read/Write DMA Access to Memory Card %c not implemented!\n", 'A' + Channel);
			break;

		case 3:		//unknown
			printf(".EXI: Unknown DMA Access to Memory Card %c\n", 'A' + Channel);
		}
	}
	else
	{
		//immediate access
		switch((exi.cr[Channel] & EXI_CR_RW) >> 2)
		{
		case 0:
			//read

			//figure out the command and readjust our buffer position as needed
			switch((WriteBuff[0] & 0xFF000000) >> 24)
			{
				case 0x00:		//Card Identification
					exi.data[Channel] = MemCardBlocks / 16;
					printf(".EXI: Memory Card %c identified as having %d blocks (%d megabits)\n", 'A' + Channel, MemCardBlocks, MemCardBlocks / 16);
					break;

				case 0x52:		//Read Block
					if(WriteBuffPtr == 0)
					{
						//convert our offset and read the data
						Offset = MemCard_ConvertOffset(WriteBuff[0], WriteBuff[1]);
						Offset = Offset + (WriteBlockCount * 4);
						exi.data[Channel] = *(u32 *)&MemCardData[Channel][Offset];
						WriteBlockCount++;

//						printf(".EXI: Memory Card %c Read Data %08X\n", 'A' + Channel, Offset);
					}
					MemCardStatus[Channel] |= 0x40000000;
					break;

				case 0x83:		//Get Card Status
					exi.data[Channel] = MemCardStatus[Channel];		//signal ready
					printf(".EXI: Memory Card %c Status %08X\n", 'A' + Channel, MemCardStatus[Channel]);
					break;

				case 0x85:		//Get ID
					exi.data[Channel] = 0xC221;
					printf(".EXI: Memory Card ID %04X returned\n", exi.data[Channel]);
					break;

				case 0xF2:
					printf(".EXI: Memory Card %c Write Block Read?\n", 'A' + Channel);
					break;

				default:
					printf(".EXI: Memory Card %c Unknown Read Command 0x%02X\n", 'A' + Channel, (WriteBuff[0] & 0xFF000000) >> 24);
			}
			break;

		case 1:
			//write the data to our buffer
			WriteBuff[WriteBuffPtr] = exi.data[Channel];

			//figure out the command and readjust our buffer position as needed
			switch((WriteBuff[0] & 0xFF000000) >> 24)
			{
				case 0x00:		//Card Identification
					break;

				case 0x52:		//Read Block
					WriteBuffPtr++;
					if(WriteBuffPtr >= 3)
						WriteBuffPtr = 0;
					WriteBlockCount = 0;
					break;

				case 0x83:		//Get Card Status
					WriteBuffPtr = 0;
					printf(".EXI: Memory Card %c Get Card Status Requested\n", 'A' + Channel);
					break;

				case 0x85:		//Get ID
					WriteBuffPtr = 0;
					printf(".EXI: Memory Card %c Get ID Requested\n", 'A' + Channel);
					break;

				case 0x89:		//Clear Card Status
					MemCardStatus[Channel] |= MCSTATUS_READY;
					WriteBuffPtr = 0;
					MemCardInterruptSet[Channel] = 0;
					printf(".EXI: Memory Card %c Clear Card Status Requested\n", 'A' + Channel);
					break;

				case 0xF1:		//Erase Sector
					WriteBuffPtr = 0;

					//wipe out our data
					WriteBuff[1] = 0;
					Offset = MemCard_ConvertOffset(WriteBuff[0], WriteBuff[1]);
					memset(&MemCardData[Channel][Offset], 0, 0x2000);
					printf(".EXI Memory Card %c Erase Sector %08X\n", 'A' + Channel, Offset);
					WriteBuffPtr = 0;
					MemCardStatus[Channel] |= MCSTATUS_BUSY;
					MemCardStatus[Channel] &= ~MCSTATUS_READY;
					MemCardBusy[Channel] = MCSTATUS_BUSY;
					MemCardBusy[2] = MemCardBusy[0] | MemCardBusy[1];
					MemCardErasing[Channel] = 200;
					break;

				case 0xF2:		//Write Block
					if(WriteBuffPtr == 0)
					{
						WriteBuffPtr++;
						WriteBlockCount = 0;
					}
					else if(WriteBuffPtr == 1)
						WriteBuffPtr++;
					else
					{
						//move to our next block count, if we write 0x80 bytes then
						//we finished the command
						WriteBlockCount++;
						if(WriteBlockCount == (0x80 / 4))
							WriteBuffPtr = 0;

						//write the data out
						Offset = MemCard_ConvertOffset(WriteBuff[0], WriteBuff[1]);
						Offset = Offset + (WriteBlockCount * 4);
						*(u32 *)&MemCardData[Channel][Offset] = BSWAP32(exi.data[Channel]);

						MemCardInterruptSet[Channel] = 1;
						MemCardStatus[Channel] &= ~MCSTATUS_BUSY;
						MemCardBusy[Channel] = 0;
						MemCardBusy[2] = MemCardBusy[0] | MemCardBusy[1];
						MemCardStatus[Channel] |= MCSTATUS_READY;

//						printf(".EXI: Memory Card %c Write Block %08X\n", 'A' + Channel, Offset);
					}
					break;

				case 0xF4:		//Erase Card
					memset(MemCardData[Channel], 0, MemCardSize);
					WriteBuffPtr = 0;
					MemCardErasing[Channel] = 200;
					MemCardStatus[Channel] |= MCSTATUS_BUSY;
					MemCardBusy[Channel] = MCSTATUS_BUSY;
					MemCardBusy[2] = MemCardBusy[0] | MemCardBusy[1];
					MemCardStatus[Channel] &= ~MCSTATUS_READY;
//					printf(".EXI Memory Card %c Erase Card\n", 'A' + Channel, Offset);
					break;

				default:
					WriteBuffPtr = 0;
					printf(".EXI: Memory Card %c Unknown Write Command 0x%02X\n", 'A' + Channel, (WriteBuff[0] & 0xFF000000) >> 24);
			}

			break;

		case 2:
			//read/write
			printf(".EXI: Memory Card IMM Read/Write not implemented for 0x%08X!\n", exi.data[0]);
			break;

		case 3:
			//unknown
			printf(".EXI: Memory Card IMM unknown not implemented!\n");
		}
	}

	if(MemCardInterruptSet[Channel])
		exi.csr[Channel] |= EXI_CSR_EXIINT;
	else
		exi.csr[Channel] &= ~EXI_CSR_EXIINT;
}

void MemCard_Checksum(u16 *buf, int num, u16 *c1, u16 *c2)
{
    int i;
    *c1 = 0;*c2 = 0;
    for (i = 0; i < (num >> 1); i++)
    {
        *c1 += BSWAP16(buf[i]);
        *c2 += BSWAP16(~buf[i]);
    }
    if (*c1 == 0xffff)
        *c1 = 0;

    if (*c2 == 0xffff)
        *c2 = 0;

	*c1 = BSWAP16(*c1);
	*c2 = BSWAP16(*c2);
}

void MemCard_Update()
{
	if(MemCardBusy[0])
	{
		if(!MemCardErasing[0])
		{
			printf("Memory Card A Clearing Busy Status\n");
			MemCardInterruptSet[0] = 1;
			exi.csr[0] |= EXI_CSR_EXIINT;

			MemCardStatus[0] &= ~MCSTATUS_BUSY;
			MemCardStatus[0] |= MCSTATUS_READY;
			MemCardBusy[0] = 0;
			MemCardBusy[2] = MemCardBusy[0] | MemCardBusy[1];
		}
		else
		{
			MemCardErasing[0]--;
		}
	}

	if(MemCardBusy[1])
	{
		if(!MemCardErasing[1])
		{
			printf("Memory Card B Clearing Busy Status\n");
			MemCardInterruptSet[1] = 1;
			exi.csr[1] |= EXI_CSR_EXIINT;

			MemCardStatus[1] &= ~MCSTATUS_BUSY;
			MemCardStatus[1] |= MCSTATUS_READY;
			MemCardBusy[1] = 0;
			MemCardBusy[2] = MemCardBusy[0] | MemCardBusy[1];
		}
		else
		{
			MemCardErasing[1]--;
		}
	}
}

void MemCard_Open()
{
	u32 Channel;

	for(Channel = 0; Channel < 2; Channel++)
	{
		MemCardData[Channel] = (u8 *)malloc(MemCardSize);
		memset(MemCardData[Channel], 0, MemCardSize);

		//set our size
		*(u16 *)(&MemCardData[Channel][0x22]) = BSWAP16(MemCardBlocks / 16);
		*(u16 *)(&MemCardData[Channel][0x24]) = BSWAP16(MemBlockType);

		//did you know that if the FlashID does not exist in the SRAM
		//that memory cards will fail to mount and throw an IOERROR, error -5
		//also, if the first 12 bytes of the memory card does not match
		//the FlashID of the SRAM for it's channel that the card code
		//will throw BROKEN, error -6
		//With our luck, we happen to be 0'ing out the ID and the memory card data
		MemCard_Checksum((u16 *)&MemCardData[Channel][0x0000], 0x01FC, (u16 *)&MemCardData[Channel][0x01FC], (u16 *)&MemCardData[Channel][0x01FE]);

		//empty directories
		memset(&MemCardData[Channel][0x2000], 0xFF, 0xFFA);
		memset(&MemCardData[Channel][0x4000], 0xFF, 0xFFA);

		MemCard_Checksum((u16 *)&MemCardData[Channel][0x2000], 0x1FFC, (u16 *)&MemCardData[Channel][0x3FFC], (u16 *)&MemCardData[Channel][0x3FFE]);
		MemCard_Checksum((u16 *)&MemCardData[Channel][0x4000], 0x1FFC, (u16 *)&MemCardData[Channel][0x5FFC], (u16 *)&MemCardData[Channel][0x5FFE]);

		//number of blocks free
		*(u16 *)&MemCardData[Channel][0x6006] = BSWAP16(MemCardBlocks - 5);
		*(u16 *)&MemCardData[Channel][0x8006] = BSWAP16(MemCardBlocks - 5);

		//last allocated block
		*(u16 *)&MemCardData[Channel][0x6008] = BSWAP16(4);
		*(u16 *)&MemCardData[Channel][0x8008] = BSWAP16(4);

		MemCard_Checksum((u16 *)&MemCardData[Channel][0x6004], 0x1FFC, (u16 *)&MemCardData[Channel][0x6000], (u16 *)&MemCardData[Channel][0x6002]);
		MemCard_Checksum((u16 *)&MemCardData[Channel][0x8004], 0x1FFC, (u16 *)&MemCardData[Channel][0x8000], (u16 *)&MemCardData[Channel][0x8002]);

		MemCardStatus[Channel] = MCSTATUS_BUSY | MCSTATUS_READY | MCSTATUS_UNLOCKED;
		MemCardErasing[Channel] = 0;
	}
}

void MemCard_Close()
{
	u32		Channel;

	for(Channel = 0; Channel < 2; Channel++)
		free(MemCardData[Channel]);
/*
	for(Channel = 0; Channel < 2; Channel++)
	{
		sprintf(MemFile, "%s\\MemCard%c-Good.raw", ProgramDirectory, 'A' + Channel);
		fHandle = CreateFile(MemFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if(fHandle != INVALID_HANDLE_VALUE)
		{
			WriteFile(fHandle, MemCardData[Channel], 0x2000 * 128, &BytesWritten, 0);
			CloseHandle(fHandle);
		}
		free(MemCardData[Channel]);
	}
*/
}