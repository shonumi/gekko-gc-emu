// hle_general.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "dvd/realdvd.h"
#include "hle_func.h"
#include "hle_general.h"
#include <string>
#include <cassert>
#include <vector>
#include "powerpc/cpu_core_regs.h"

using namespace std;

u32		__OSPhysicalContext;
u32		__OSCurrentContext;
u32		__OSDefaultThread;     
u32		dvdfilehandle[128];
u32		filehandle_ptr[128];

// Desc: Standard C
////////////////////////////////////////////////////////////
/*
HLE(memcpy)
{
	memcpy(HLE_PARAM_PTR_0, HLE_PARAM_PTR_1, HLE_PARAM_INT_2);
}

HLE(memset)
{
	memset(HLE_PARAM_PTR_0, HLE_PARAM_INT_1, HLE_PARAM_INT_2);
}

HLE(strchr)
{
	u32 Ret;

	Ret = (u32)strchr((char *)HLE_PARAM_PTR_0, HLE_PARAM_INT_1);
	if(Ret)
	{
		Ret -= (u32)RAM;
		Ret = Ret | (HLE_PARAM_INT_0 & ~RAM_MASK);
	}
	HLE_RETURN(Ret);
}

HLE(strcmp)
{
	HLE_RETURN(strcmp((char *)HLE_PARAM_PTR_0, (char *)HLE_PARAM_PTR_1));
}

HLE(strcpy)
{
	strcpy((char *)HLE_PARAM_PTR_0, (char *)HLE_PARAM_PTR_1);
}

HLE(strlen)
{
	HLE_RETURN(strlen((char *)HLE_PARAM_PTR_0));
}

HLE(sinfcosf)
{
	if(Memory_Read32(ireg.PC + 0xB0) == 0x4BFFFF49)
	{
		PS0(1) = cosf(PS0(1));
	}else if(Memory_Read32(ireg.PC + 0xB0) == 0x4BFFFDB5)
	{
		PS0(1) = sinf(PS0(1));
	}
}

// Desc: Interrupt
////////////////////////////////////////////////////////////

HLE(OSEnableInterrupts)
{
	GPR(3) = ireg.MSR;
	GPR(4) = GPR(3) | 0x8000;
	ireg.MSR = GPR(4);
	GPR(3) = (GPR(3) >> 15) & 0x01;
}

HLE(OSDisableInterrupts)
{
    u32 prev = ireg.MSR;
    ireg.MSR &= ~0x8000;
    GPR(3) = (prev >> 15) & 1;
}

HLE(OSRestoreInterrupts)
{
	u32 temp = ireg.MSR;
    if(GPR(3)) ireg.MSR |= 0x8000;
    else ireg.MSR &= ~0x8000;
    GPR(3) = (temp >> 15) & 0x01;
}

// Desc: Console
////////////////////////////////////////////////////////////
*/
HLE(OSReport)
{
	char OutBuff[512], InBuff[512], Temp1[512], Temp2[512], Temp3[512];
	char *Offset1, *Offset2;
	int  CurRegister;
	u32	 i;

	//copy our string
	i =-4;
	do
	{
		i+=4;

		if(i >= 512)
		{
			InBuff[511] = 0;
			break;
		}

		*(u32 *)&InBuff[i] = BSWAP32(Memory_Read32(ireg.gpr[3] + i));
	} while(InBuff[i] && InBuff[i+1] && InBuff[i+2] && InBuff[i+3]);

	//sprintf(InBuff, "%s", &Mem_RAM[ireg.gpr[3]&0x0FFFFFFF]);

	CurRegister = 4;
	Offset1 = strstr(InBuff, "%");
	if(!Offset1)
	{
		LOG_NOTICE(TOS_REPORT, "%s", InBuff);
		return;
	}


	strncpy(OutBuff, InBuff, Offset1 - InBuff);
	OutBuff[Offset1 - InBuff] = 0;
	while(Offset1)
	{
		Offset2 = strstr((Offset1+1), "%");
		if(Offset2 == 0)
			Offset2 = &InBuff[strlen(InBuff)];

		strncpy(Temp1, Offset1, Offset2 - Offset1);
		Temp1[Offset2 - Offset1] = 0;

		if((Temp1[1] | 0x20) == 's')
		{
			//copy the string to temp3
			i = -4;
			do
			{
				i += 4;
				*(u32 *)&Temp3[i] = BSWAP32(Memory_Read32(ireg.gpr[CurRegister] + i));
			} while(Temp3[i] && Temp3[i+1] && Temp3[i+2] && Temp3[i+3]);

			sprintf(Temp2, Temp1, Temp3);
		}
		else
			sprintf(Temp2, Temp1, ireg.gpr[CurRegister]);

		strcat(OutBuff, Temp2);
		Offset1++;
		Offset1 = strstr(Offset1, "%");
		CurRegister++;
	}

	LOG_NOTICE(TOS_REPORT, "%s", OutBuff);
}
/*
HLE(DEMOWinLogPrintf)
{
	printf(">%s",&RAM[GPR(4)&0x0FFFFFFF]);
}
*/
HLE(DBPrintf)
{
	char OutBuff[512], InBuff[512], Temp1[512], Temp2[512], Temp3[512];
	char *Offset1, *Offset2;
	int  CurRegister;
	u32  i;

	//copy our string
	i = -4;
	do
	{
		i += 4;

		if(i >= 512)
		{
			InBuff[511] = 0;
			break;
		}

		*(u32 *)&InBuff[i] = BSWAP32(Memory_Read32(ireg.gpr[3] + i));
	} while(InBuff[i] && InBuff[i+1] && InBuff[i+2] && InBuff[i+3]);
//	sprintf(InBuff, "%s", &Mem_RAM[ireg.gpr[3]&0x0FFFFFFF]);

	CurRegister = 4;
	Offset1 = strstr(InBuff, "%");
	if(!Offset1)
	{
		LOG_NOTICE(TOS_HLE, "%s", InBuff);
		return;
	}


	strncpy(OutBuff, InBuff, Offset1 - InBuff);
	OutBuff[Offset1 - InBuff] = 0;
	while(Offset1)
	{
		Offset2 = strstr((Offset1+1), "%");
		if(Offset2 == 0)
			Offset2 = &InBuff[strlen(InBuff)];

		strncpy(Temp1, Offset1, Offset2 - Offset1);
		Temp1[Offset2 - Offset1] = 0;

		if((Temp1[1] | 0x20) == 's')
		{
			//copy the string to temp3
			i = -4;
			do
			{
				i += 4;
				*(u32 *)&Temp3[i] = BSWAP32(Memory_Read32(ireg.gpr[CurRegister] + i));
			} while(Temp3[i] && Temp3[i+1] && Temp3[i+2] && Temp3[i+3]);

			sprintf(Temp2, Temp1, Temp3);
		}
		else
			sprintf(Temp2, Temp1, ireg.gpr[CurRegister]);

		strcat(OutBuff, Temp2);
		Offset1++;
		Offset1 = strstr(Offset1, "%");
		CurRegister++;
	}

	LOG_NOTICE(TOS_HLE, "%s", OutBuff);
}

HLE(OSPanic)
{
	u32	i;
	u8 InBuff[1024];

	//copy our string
	i = -4;
	do
	{
		i += 4;
		if(i >= 1024)
		{
			InBuff[1023] = 0;
			break;
		}

		*(u32 *)&InBuff[i] = BSWAP32(Memory_Read32(ireg.gpr[5] + i));
	} while(InBuff[i] && InBuff[i+1] && InBuff[i+2] && InBuff[i+3]);
	LOG_NOTICE(TOS_HLE, "%s",InBuff);
}
/*
// Desc: OS Context
////////////////////////////////////////////////////////////

HLE(OSInitContext)
{
    OSContext *context = (OSContext*)(&RAM[GPR(3)&0x0FFFFFFF]);

    context->srr0 = BSWAP32(GPR(4));
    context->gpr[1] = BSWAP32(GPR(5));
    context->srr1 = BSWAP32(MSR_EE|MSR_ME|MSR_IR|MSR_DR|MSR_RI);
    
    context->cr = 0;
    context->xer = 0;

    for(int i=0;i<8;i++) context->gqr[i] = 0;

    HLE_FUNCTION(OSClearContext);

    for(int i=3;i<32;i++) context->gpr[i] = 0;
    
	context->gpr[2] = BSWAP32(ireg.gpr[2]);
    context->gpr[13] = BSWAP32(ireg.gpr[13]);
}

HLE(__OSContextInit)
{
	__OSCurrentContext = 0;
	Memory_Write32(OS_DEFAULT_THREAD,__OSCurrentContext);

	ireg.MSR |= (MSR_FP|MSR_RI);
}

HLE(OSSetCurrentContext)
{
	__OSCurrentContext =  GPR(3);
	__OSPhysicalContext = GPR(3) & 0x0FFFFFFF;
	
	Memory_Write32(OS_CURRENT_CONTEXT,  __OSCurrentContext);
    Memory_Write32(OS_PHYSICAL_CONTEXT, __OSPhysicalContext);

	OSContext *context = (OSContext*)(&RAM[__OSPhysicalContext]);

	if(__OSCurrentContext == __OSDefaultThread)
	{	
		context->srr1 |= BSWAP32(MSR_FP);
	}
	else
	{
		context->srr1 |= BSWAP32(MSR_FP);
        ireg.MSR |= MSR_FP;
	}

	ireg.MSR |= MSR_RI; 
}

HLE(OSGetCurrentContext)
{
   GPR(3) = __OSCurrentContext;
}

HLE(OSSaveContext)
{
    OSContext *context = (OSContext*)(&RAM[GPR(3)&0x0FFFFFFF]);

    HLE_FUNCTION(OSSaveFPUContext);

    for(int i=13;i<32;i++) context->gpr[i] = BSWAP32(ireg.gpr[i]);

    for(int i=1; i<8; i++) context->gqr[i] = BSWAP32(GQR(i));

    context->cr = BSWAP32(ireg.CR);
    context->lr = BSWAP32(LR);
    context->ctr = BSWAP32(CTR);
    context->xer = BSWAP32(XER);
    context->srr0 = context->lr;
    context->srr1 = BSWAP32(ireg.MSR);

    context->gpr[1] = BSWAP32(SP);
    context->gpr[2] = BSWAP32(ireg.gpr[2]);
    context->gpr[3] = BSWAP32(ireg.gpr[0]=1);

    GPR(3) = 0;
}

HLE(OSLoadContext)
{
	OSContext *context = (OSContext*)(&RAM[GPR(3)&0x0FFFFFFF]);

    ireg.gpr[0] = BSWAP32(context->gpr[0]);
    ireg.gpr[1] = BSWAP32(context->gpr[1]);  
    ireg.gpr[2] = BSWAP32(context->gpr[2]); 
    
    HLE_FUNCTION(OSLoadFPUContext);

    for(int i=1; i<8; i++) GQR(i) = BSWAP32(context->gqr[i]);

    u16 state = (context->state >> 8)|(context->state << 8);
    if(state & OS_CONTEXT_STATE_EXC)
    {
        state &= ~OS_CONTEXT_STATE_EXC;
        context->state = (state >> 8)|(state << 8);
        for(int i=5; i<32; i++) ireg.gpr[i] = BSWAP32(context->gpr[i]);
    }
	else { for(int i=13; i<32; i++) ireg.gpr[i] = BSWAP32(context->gpr[i]); }

    ireg.CR = BSWAP32(context->cr);
    LR  = BSWAP32(context->lr);
    CTR = BSWAP32(context->ctr);
    XER = BSWAP32(context->xer);

    SRR0 = BSWAP32(context->srr0);
    SRR1 = BSWAP32(context->srr1);

    ireg.gpr[3] = BSWAP32(context->gpr[3]);
    ireg.gpr[4] = BSWAP32(context->gpr[4]);
}

HLE(OSClearContext)
{
	OSContext *context = (OSContext*)(&RAM[GPR(3)&0x0FFFFFFF]);

    context->mode = 0;
    context->state = 0;

    if(GPR(3) == __OSDefaultThread) 
    {
        __OSDefaultThread = NULL;
        Memory_Write32(OS_DEFAULT_THREAD,__OSDefaultThread);
    }
}

HLE(OSLoadFPUContext)
{
	GPR(4) = GPR(3);

	OSContext *context = (OSContext*)(&RAM[GPR(4)&0x0FFFFFFF]);

	context->fpscr = BSWAP32(ireg.FPSCR);

	for(int i=0;i<32;i++)
	{
		if (PSE) { 	*(u64 *)(&context->psr[i]) = BSWAP64(PS1_B(i)); } 
		*(u64 *)(&context->fpr[i]) = BSWAP64(PS0_B(i));
	}
}

HLE(OSSaveFPUContext)
{
	GPR(4) = GPR(3);
	OSContext *context = (OSContext*)(&RAM[GPR(4)&0x0FFFFFFF]);

	ireg.FPSCR = BSWAP32(context->fpscr);

	for(int i=0;i<32;i++)
	{
		if (PSE) { PS1_B(i) = BSWAP64(*(u64 *)(&context->psr[i])); } 
		PS0_B(i) = BSWAP64(*(u64 *)(&context->fpr[i]));
	}
}

HLE(OSFillFPUContext)
{
	OSContext *context = (OSContext*)(&RAM[GPR(3)&0x0FFFFFFF]);

	ireg.MSR |= MSR_FP;
	context->fpscr = BSWAP32(ireg.FPSCR);

	for(int i=0;i<32;i++)
	{
		*(u64 *)(&context->fpr[i]) = BSWAP64(PS0_B(i));
		if (PSE) { *(u64 *)(&context->psr[i]) = BSWAP64(PS1_B(i)); }
	}
}

// Desc: DVD
	////////////////////////////////////////////////////////////
*/
HLE(DVDOpen)
{
//#define DO_DVD_HACK
#ifdef DO_DVD_HACK

	char filepath[1024];
	char ext[64];
	char str[256], *c;

	memset(filepath,0,sizeof(filepath));
	sprintf(filepath,"%s",&RAM[GPR(3)&RAM_MASK]);
	printf("DVDOpen: %s\n",filepath);


	c = strrchr (filepath, '/');
	if (c)
		strcpy (str, c + 1);
	else
	{
		c = strrchr (filepath, '\\');
		if (c)
			strcpy (str, c + 1);
		else
			strcpy (str, filepath);
	}

	c = strrchr (str, '.');
	if (c)
		strcpy (ext, c + 1);
	
	if (0 == strcmp (ext, "rel"))
	{
		printf("REL FILE LOADED INTO RAM - MUST RESCAN BOUNDS\n");
	}

	HLE_ExecuteLowLevel();
#else
	char	filepath[1024];
	u32		i;

	//memset(filepath,0,sizeof(filepath));

	//figure out how much to copy and turn around
	i = 0;
	do
	{
		*(u32 *)&filepath[i] = BSWAP32(Memory_Read32(GPR(3) + i));
		i+=4;
	} while(filepath[i] && filepath[i+1] && filepath[i+2] && filepath[i+3]);

	u32 dvdfilehandle = 0;
	dvdfilehandle = dvd::RealDVDOpen(filepath);
	if(dvdfilehandle==0)
	{
		printf("DVDOpen: Error opening %s\n",filepath);
		GPR(3) = 0;
		return;
	}

	setDVDFileHandle(GPR(4),dvdfilehandle);

	Memory_Write32(GPR(4) + sizeof(DVDCommandBlock) + 4, dvd::RealDVDGetFileSize(dvdfilehandle));
	Memory_Write32(GPR(4) + sizeof(DVDCommandBlock) + 8, 0);

	printf("DVDOpen: %s, GPR3 = %08x\n",filepath,GPR(3));
	//GPR(3) = 1;
#endif
}

HLE(DVDClose)
{
	u32 dvdfilehandle = 0;
	dvdfilehandle = getDVDFileHandle(GPR(3));
	delDVDFileHandle(GPR(3));
	if(dvdfilehandle != 0) 
	{
		dvd::RealDVDClose(dvdfilehandle);
		printf("DVDClose: Successfull\n");
		GPR(3) = 1;
		return;
	}

	printf("DVDClose: Failed\n");
	GPR(3) = 0;
}

HLE(DVDReadPrio)
{
	u32 dvdfilehandle = 0;
	u32	i;
	u32 InData;

	dvdfilehandle = getDVDFileHandle(GPR(3));
	if(dvdfilehandle!=0)
	{
//		DVDFileInfo *dvdfileinfo = (DVDFileInfo*)(&RAM[GPR(3)&RAM_MASK]);
		for(i = 0; i < (GPR(5) >> 2); i++)
		{
			dvd::RealDVDRead(dvdfilehandle, &InData, 4);
			Memory_Write32(GPR(4) + (i * 4), InData);
		}

		for(i = (i * 4); i < GPR(5); i++)
		{
			dvd::RealDVDRead(dvdfilehandle, &InData, 1);
			Memory_Write8(GPR(4) + i, InData);
		}

		printf("DVDReadPrio(0x%08X,0x%08X,0x%08X,0x%08X,0x%08X)\n",GPR(3),GPR(4),GPR(5),GPR(6),GPR(7));
		GPR(3) = GPR(5);
		return;
	}

	printf("DVDReadPrio(0x%08X,0x%08X,0x%08X,0x%08X,0x%08X) FAILED!\n",GPR(3),GPR(4),GPR(5),GPR(6),GPR(7));
	GPR(3) = -1;
}
