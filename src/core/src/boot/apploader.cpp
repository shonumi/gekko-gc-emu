#include "common.h"
#include "memory.h"
#include "powerpc/cpu_core.h"
#include "dvd/realdvd.h"
#include "boot/bootrom.h"
#include "hle/hle.h"
#include "hle/hle_func.h"

u32 AppLoaderPrint() {
	u32	    i;
	char	msg[1000];

	for(i = 0; ; i++) {
		msg[i] = Memory_Read8(ireg_GPR(3) + i);
		if(!msg[i]) {
			break;
        }
	}
	LOG_NOTICE(TBOOT, msg, ireg_GPR(4), ireg_GPR(5), ireg_GPR(6));
    return 0;
}

bool Boot_AppLoader(u32 *AppHeader)
{
	u32		i;
	u32		OldPC;
	u32		AppLoaderDump[] =
	{
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)ValidateRegLayout,	//Ptr for validate register layout
		0x3E208131,			//addis r17, 0, 0x8131
		0x3A310000,			//addi r17, r17, 0x0000 - filled in
		0x388D0000,			//addi r4, r13, 0
		0x38710000,			//addi r3, r17, 0		//boot string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8, Skip the ptr value (hle calls are 3 ops)
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x3C608130,			//addis r3, 0, 0x8130		//start loop
		0x38630004,			//addi r3, r3, 4
		0x38830004,			//addi r4, r3, 4
		0x38A40004,			//addi r5, r4, 4
		0x38c50004,			//addi r6, r5, 4
		0x7DA903A6,			//mtspr 9, r13
		0x4E800421,			//bcctr 20, 0
		0x388D0000,			//addi r4, r13, 0
		0x38710021,			//addi r3, r17, 0x21		//complete string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x3C608130,			//addis r3, 0, 0x8130
		0x3C804E80,			//addis r4, 0, 0x4E80
		0x38840020,			//addi r4, r4, 0x0020
		0x90830000,			//stw r4, r3, 0
		0x80830004,			//lwz r4, 0x04(r3)
		0x81E30008,			//lwz r15, 0x08(r3)
		0x8203000C,			//lwz r16, 0x0C(r3)
		0x38710000,			//addi r3, r17, 0		//boot string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x3C608130,			//addis r3, 0, 0x8130
		0x7C8903A6,			//mtspr 9, r4
		0x4E800421,			//bcctr 20, 0
		0x388E0000,			//addi r4, r14, 0
		0x38710021,			//addi r3, r17, 0x21		//complete string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x388F0000,			//addi r4, r15, 0
		0x38710000,			//addi r3, r17, 0		//boot string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x3C608130,			//addis r3, 0, 0x8130		//start loop
		0x38630004,			//addi r3, r3, 4
		0x38830004,			//addi r4, r3, 4
		0x38A40004,			//addi r5, r4, 4
		0x38c50004,			//addi r6, r5, 4
		0x7DE903A6,			//mtspr 9, r15
		0x4E800421,			//bcctr 20, 0
		0x3A430000,			//addi r18, r3, 0		//copy r3
		0x3CE08130,			//addis r7, 0, 0x8130
		0x80870008,			//lwz r4, r7, 0x08		//r4 = bytes
		0x2C040000,			//cmpi cr0, 0, r4, 0
		0x41820048,			//bc 12, 2, 0x48		//bypass load if r4 == 0
		0x3D00CC00,			//addis r8, 0, 0xCC00
		0x39086000,			//addi r8, r8, 0x6000
		0x90880010,			//stw r4, r8, 0x10		//store lle dvd number of bytes
		0x80A70004,			//lwz r5, r7, 0x04		//r5 = pos
		0x90A80014,			//stw r5, r8, 0x14		//store lle dvd memory pos
		0x80E7000C,			//lwz r7, r7, 0x0C		//r7 - dvd offset
		0x38C70000,			//addi r6, r7, 0		//r6 - dvd offset
		0x7CE71670,			//srawi r7, r7, 2		//r7 - dvd offset >> 2
		0x90E8000C,			//stw r7, r8, 0x0C		//store lle dvd offset
		0x3D20A800,			//addis r9, 0, 0xA800	//read dvd command
		0x91280008,			//stw r9, r8, 0x08		//store lle dvd cmd 0
		0x39200001,			//addi r9, 0, 0x01		//execute command
		0x9128001C,			//stw r9, r8, 0x1C		//send execute to lle dvd
		0x3871004F,			//addi r3, r17, 0x4F	//dvd string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x2C120000,			//cmpi cr0, 0, r18, 0
		0x4082FF88,			//bc 4, 2, -0x78			//loop if r17 != 0
		0x388F0000,			//addi r4, r15, 0
		0x38710021,			//addi r3, r17, 0x21		//complete string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x38900000,			//addi r4, r16, 0
		0x38710000,			//addi r3, r17, 0		//boot string
		0x0C000000,			//HLE Call
		0x48000008,			//b 8
		(DWORD)AppLoaderPrint,	//Ptr for Printf
		0x7E0903A6,			//mtspr 9, r16
		0x4E800421,			//bcctr 20, 0
		0x3A430000,			//addi r18, r3, 0		//copy r3
		0x3871001F,			//addi r3, r17, 0x1F		//complete string
		0x0C000000,			//HLE Call
		0x48000000,			//b 0
		(DWORD)AppLoaderPrint	//Ptr for Printf
	};

	char BootMsgs[] = "Booting Apploader function %08X\n\0Completed booting of Apploader function %08X\n\0AppLoader: %d bytes loaded to %08X from dvd offset %08X\n\0";

	//boot based on the start entry in the header
//	Bootrom();
	cpu->Open(0x81310000);

	//set some registers
	set_ireg_gpr(I_SP, 0x816FFFFC);			//stack pointer

	//set pointers for functions
	set_ireg_GPR(3, 0x81300004);
	set_ireg_GPR(4, 0x81300008);
	set_ireg_GPR(5, 0x8130000C);
	set_ireg_spr(I_LR, 0);

	//disable the hardware interrupts
	set_ireg_MSR(ireg_MSR() & ~MSR_BIT_EE);

	set_ireg_GPR(13, BSWAP32(AppHeader[4]));
	for(i=0; i < (sizeof(AppLoaderDump) / 4); i++)
		Memory_Write32(0x81310000 + (i * 4), AppLoaderDump[i]);

	for(i = 0; i <= sizeof(BootMsgs); i++)
		Memory_Write8(0x81310000 + sizeof(AppLoaderDump) + i, BootMsgs[i]);

//	memcpy(&Mem_RAM[(0x81310000 & RAM_MASK) + sizeof(AppLoaderDump)], BootMsgs, sizeof(BootMsgs));
//	*(u16 *)&RAM[(0x81310000 & RAM_MASK) + 18] = BSWAP16((u16)sizeof(AppLoaderDump));
	Memory_Write16(0x81310000 + 18, sizeof(AppLoaderDump));

	while(ireg_PC())
	{
		if(!cpu->pause)
			cpu->ExecuteInstruction();

		if(ireg_PC() == (0x81310000 + sizeof(AppLoaderDump) - 8))
		{
//			if(cpu->opcode == AppLoaderDump[(sizeof(AppLoaderDump) / 4) - 2])
			{
				break;
			}
		}

		OldPC = ireg_PC();
#pragma todo("do we need this WIN_Msg??")		
		//WIN_Msg();
	}

	//enable the hardware interrupts
	set_ireg_MSR(ireg_MSR() | MSR_BIT_EE);

	//set the PC as to where we are running
	set_ireg_PC(ireg_GPR(18));
	set_ireg_GPR(3, ireg_GPR(18));
//	cpu->pPC = MEMPTR32(ireg_PC());

	LOG_NOTICE(TBOOT, "Apploader initialized at entry point %08X ok",ireg_PC());
	return true;
}
