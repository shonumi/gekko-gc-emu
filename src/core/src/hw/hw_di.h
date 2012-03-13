// hw_CP.h
// (c) 2005,2006 Gekko Team

#ifndef _HW_DI_HEADER_
#define _HW_DI_HEADER_

#include "common.h"

typedef struct t_sDI
{
	u32	CmdBuff[3];
	u32 DMAMemory;
	u32 DMALength;

	u32	cvr;
	u32 sr;
	u32 cr;

	u32 IMMBuf;
	u32	cfg;
} sDI;

extern sDI hw_di;

//potential command values
#define DI_CMD_INQUIRY			0x12
#define DI_CMD_READDATA			0xA8
#define DI_CMD_SEEK				0xAB
#define DI_CMD_REQUESTERR		0xE0
#define DI_CMD_PLAYAUDIO		0xE1
#define DI_CMD_REQAUDIOSTAT		0xE2
#define DI_CMD_STOPMOTOR		0xE3
#define DI_CMD_DVDAUDIO			0xE4

//sub flags for commands
#define DI_CMD_READDATA_SECTOR	0x00
#define DI_CMD_READDATA_DISKID	0x40

#define DI_CMD_DVDAUDIO_DISABLE	0x00
#define DI_CMD_DVDAUDIO_ENABLE	0x01


//values from YAGCD
#define DI_SR		0xCC006000
#define DI_CVR		0xCC006004
#define DI_CMD0		0xCC006008
#define DI_CMD1		0xCC00600C
#define DI_CMD2		0xCC006010
#define DI_DMAMEM	0xCC006014
#define DI_DMALEN	0xCC006018
#define DI_CR		0xCC00601C
#define DI_IMMBUF	0xCC006020
#define DI_CFG		0xCC006024

//flags for DI_SR
#define DI_SR_BRKINT		(1 << 6)
#define DI_SR_BRKINTMASK	(1 << 5)
#define DI_SR_TCINT			(1 << 4)
#define DI_SR_TCINTMASK		(1 << 3)
#define DI_SR_DEINT			(1 << 2)
#define DI_SR_DEINTMASK		(1 << 1)
#define DI_SR_BRK			(1 << 0)

//flags for DI_CVR
#define DI_CVR_CVRINT		(1 << 2)
#define DI_CVR_CVRINTMASK	(1 << 1)
#define DI_CVR_CVR			(1 << 0)

//flags for DI_CR
#define DI_CR_RW			(1 << 2)
#define DI_CR_DMA			(1 << 1)
#define DI_CR_TSTART		(1 << 0)

void DI_Open(void);
void DI_Close(void);

u8		__fastcall	DI_Read8(u32 addr);
void	__fastcall	DI_Write8(u32 addr, u32 data);
u16		__fastcall	DI_Read16(u32 addr);
void	__fastcall	DI_Write16(u32 addr, u32 data);
u32		__fastcall	DI_Read32(u32 addr);
void	__fastcall	DI_Write32(u32 addr, u32 data);

#endif