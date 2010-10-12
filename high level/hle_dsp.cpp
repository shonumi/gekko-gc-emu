#include "../emu.h"
#include "../low level/hardware core/hw_dsp.h"
#include "../low level/hardware core/hw_pi.h"
#include "../low level/hardware core/hw_ai.h"
#include "hle_dsp.h"

int	DSPucode;

#define MESSAGEQUEUESIZE 256
u32 messagequeue[MESSAGEQUEUESIZE];
int messagequeue_readloc;
int messagequeue_writeloc;

struct {
	u32 command;
	u32 DMA_RAMaddr;
	u32 DMA_IRAMaddr;
	u32 DMA_size;
	u32 DMA_DRAMaddr;
	u32 DMA_execaddr;
	
	u32 * parambuf;

	int paramsleft;
} ucode_loader ;

void dsphle_init(void) {
	DSPucode = DSPUCODE_LOADER;
	ucode_loader.paramsleft=0; /* no parameters expected */
	messagequeue_readloc=0;
	messagequeue_writeloc=1;
}

void ucode_loader_parse(u32 message) {
	//printf ("ucode_loader_parse: %08x\n");
	if (ucode_loader.paramsleft>0) {
		*ucode_loader.parambuf = message;
		ucode_loader.parambuf++;
		ucode_loader.paramsleft--;
	} else {
		ucode_loader.command=message;

		switch (message) {
		case 0x80f3a001: /* RAM addr */
			ucode_loader.paramsleft=1;
			ucode_loader.parambuf=&ucode_loader.DMA_RAMaddr;
			break;
		case 0x80f3c002:
			ucode_loader.paramsleft=1;
			ucode_loader.parambuf=&ucode_loader.DMA_IRAMaddr;
			break;
		case 0x80f3a002:
			ucode_loader.paramsleft=1;
			ucode_loader.parambuf=&ucode_loader.DMA_size;
			break;
		case 0x80f3b002:
			ucode_loader.paramsleft=1;
			ucode_loader.parambuf=&ucode_loader.DMA_DRAMaddr;
			break;
		case 0x80f3d001:
			ucode_loader.paramsleft=1;
			ucode_loader.parambuf=&ucode_loader.DMA_execaddr;
			break;
		default:
			printf("Unknown message to ucode_loader: %08x\n",message);
			return;
		}
	}
	if (ucode_loader.paramsleft==0) {
		if (ucode_loader.command==0x80f3d001) {
			printf("Execute DSP ucode from RAM %08x size %8x\n",ucode_loader.DMA_RAMaddr,
				ucode_loader.DMA_size);

			/* generate checksum */
			u32 crc = GenerateCRC(
				&Mem_RAM[ucode_loader.DMA_RAMaddr & RAM_MASK],
				ucode_loader.DMA_size);
			printf("crc32=%08x\n",crc);
			switch (crc) {
			case 0x37c241aa: // Twilight Princess NTSC-J
			case 0x5d0d105e: // Wind Waker NTSC-U
			case 0xd2fdb38c: // Twlight Princess NTSC-U
				printf("Zelda: Wind Waker ucode\n");
				DSPucode=DSPUCODE_ZWW;
				ucode_zww_init();
				break;
			}
		}
	}
}

void write_msg_queue(u32 msg) {
	if (messagequeue_writeloc==messagequeue_readloc) {
		printf("msg queue overflow\n");
		return;
	}
	messagequeue[messagequeue_writeloc]=msg;
	messagequeue_writeloc++;
	if (messagequeue_writeloc >= MESSAGEQUEUESIZE) messagequeue_writeloc=0;
}

u32 peek_msg_queue(void) {
	int nextread=messagequeue_readloc+1;
	if (nextread>=MESSAGEQUEUESIZE) nextread=0;
	if (nextread==messagequeue_writeloc) return 0;
	//messagequeue_readloc=nextread;
	return messagequeue[nextread];
}

void pop_msg_queue(void) {
	int nextread=messagequeue_readloc+1;
	if (nextread>=MESSAGEQUEUESIZE) nextread=0;
	if (nextread==messagequeue_writeloc) return;
	messagequeue_readloc=nextread;
}

int is_msg_queue_empty(void) {
	int nextread=messagequeue_readloc+1;
	if (nextread>=MESSAGEQUEUESIZE) nextread=0;
	return (nextread==messagequeue_writeloc);
}

struct {
	int bufaddr;
	u32 databuf[256];

	int lenleft;

	int altmode;
} ucode_zww ;	

void ucode_zww_sendmsg(u32 msg, int irq) {
	if (irq) {
		REGDSP16(DSP_CSR)  |= DSP_CSR_DSPINT;
		dspCSRDSPInt = DSP_CSR_DSPINT;
		//PI_RequestInterrupt(PI_MASK_DSP);
	}
	write_msg_queue(msg);
}

//uint32 read_msg(

void ucode_zww_init(void) {
	/* handshake */
	ucode_zww_sendmsg(0xdcd10000,1);
	ucode_zww_sendmsg(0xf3551111,0);
	ucode_zww.lenleft=0;
	ucode_zww.altmode=0;
}

void ucode_zww_parse(u32 mesg) {
	if (ucode_zww.lenleft>0) {
		//printf("param: %08x\n",mesg);
		ucode_zww.lenleft--;
		ucode_zww.databuf[ucode_zww.bufaddr++]=mesg;

		if (ucode_zww.lenleft==0) {
			if (ucode_zww.altmode) {
				printf("altmode: a[%d]=%04x\n",(mesg>>16)&0xf,mesg&0xffff);
			} else {
				printf ("message length %d\n",ucode_zww.bufaddr);
				for (int i=0;i<ucode_zww.bufaddr;i++) {
					printf("%08x\n",ucode_zww.databuf[i]);
				}
	
				ucode_zww_processcmd(ucode_zww.databuf,ucode_zww.bufaddr);
			}
		}
	} else {
		if (!(mesg&0xffff)) {
			ucode_zww.altmode=1;
			ucode_zww.lenleft=1;
		} else {

			//printf("expecting len=%08x\n",mesg&0xffff);
			ucode_zww.lenleft=mesg&0xffff;

			ucode_zww.bufaddr=0;
		}
	}
}

void ucode_zww_processcmd(u32 * data,int size) {
	int command = (data[0]>>24) & 0x7f;

	printf("ZWW command #%d\n",command);
	switch (command) {
	case 1:
		/* DMA, setup */
		/* params unknown */

		printf("setup: DMA from %08x and %08x\n",data[2],data[3]);
		break;
	case 2:
		/* mixing? */
		/* params unknown */
		break;
	case 4:
		/* DMA in */
		/* params unknown */
		break;
	case 5:
		/* DMA out */
		/* params unknown */
		break;
	case 6:
		/* accel stuff */
		/* params unknown */
		break;
	case 7:
		/* swap memory? */
		/* params unknown */
		break;
	case 8:
		/* multiply a region */
		/* params unknown */
		break;
	case 9:
		/* copy */
		/* params unknown */
		break;
	case 13:
		/* short msg */
		/* params unknown */
		break;
	case 0:
	case 3:
	case 10:
	case 11:
	case 12:
	case 14:
	case 15:
		/* NOP */
		printf("NOP\n");

		/* needs to send command back */

		break;
	default:
		break;
	}

	ucode_zww_sendmsg(0xdcd10004,1);
	//if (command==2) ucode_zww_sendmsg(0xf3550000,0);
	//else ucode_zww_sendmsg(0xf3550000|(data[0]>>16),0);
	ucode_zww_sendmsg(0xf3550000|(data[0]>>16),0);
}