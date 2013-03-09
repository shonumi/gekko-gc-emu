/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    ucode.h
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2012-12-01
 * @brief   Base class for DSP microcode (ucode)
 *
 * @section LICENSE
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */ 

#include "ucode.h"
#include "ucode_rom.h"
#include "ucode_zelda.h"

#include "crc.h"
#include "memory.h"
#include "hw/hw_dsp.h"
#include "hw/hw_pi.h"
#include "hw/hw_ai.h"

//TODO: Might as well put this in DSPHLE
UCode* GenerateUCode(u32 crc, MailManager* mail_mngr) {
    switch(crc) {

        case UCODE_ROM:       //IROM
            return new UCode_ROM(mail_mngr);

        case 0x86E5FF9D:            //Legend of Zelda: Wind Waker (NTSC-U/PAL);
            printf("Booting Zelda Ucode\n");
            return new UCode_Zelda(mail_mngr);

	default: 
            return NULL;
    }
}          

//UCode Constructor
UCode::UCode() {
    resume_mail = false;
    upload_in_progress = true;
    ucode_steps = 0;
    crc = 0;
}

/**
 * Determines if we need to send a resume mail to CPU
 * @returns True if resume mail flag is set, false if not
 */
bool UCode::ResumeMail() {
    if(resume_mail) {
        resume_mail = false;
        return true;
    }
    return false;
}

//TODO: SendMail should be part of DSPHLE, ideally

/**
 * Sends mail from DSP to CPU
 * @param message Mail going to CPU
 * @param interrupt_req Requests interrupt from DSP if 1
 */
void UCode::SendMail(u32 message, int interrupt_req) {
    if(interrupt_req) {
        REGDSP16(DSP_CSR)  |= DSP_CSR_DSPINT;
		dspCSRDSPInt = DSP_CSR_DSPINT;
		//PI_RequestInterrupt(PI_MASK_DSP);
    }
	
    mail_man->PushMail(message);
}

/**
 * Sets ucode up to boot
 * @param message Mail sent to DSP for booting
 */
void UCode::PrepareBootUCode(u32 message) {
    switch(ucode_steps) {
        
        case 0:
            uc_loader.mram_dest_addr = message; 
            printf("MRAM Destination Address: %08x\n", uc_loader.mram_dest_addr);
            break;

        case 1:
            uc_loader.mram_size = (message & 0xFFFF); 
            printf("MRAM Size: %08x\n", uc_loader.mram_size);
            break;

        case 2:
            uc_loader.mram_dram_addr = (message & 0xFFFF); 
            printf("MRAM DRAM Address: %08x\n", uc_loader.mram_dram_addr);
            break;

        case 3:
            uc_loader.iram_mram_addr = message; 
            printf("IRAM MRAM Address: %08x\n", uc_loader.iram_mram_addr);
            break;

        case 4:
            uc_loader.iram_size = (message & 0xFFFF); 
            printf("IRAM Size: %08x\n", uc_loader.iram_size);
            break;

        case 5:
            uc_loader.iram_dest = (message & 0xFFFF); 
            printf("IRAM Destination: %08x\n", uc_loader.iram_dest);
            break;

        case 6:
            uc_loader.iram_startpc = (message & 0xFFFF);
            printf("IRAM Start PC: %08x\n", uc_loader.iram_startpc);  
            break;

        case 7:
            uc_loader.dram_mram_addr = message; 
            printf("DRAM -> MRAM Address: %08x\n", uc_loader.dram_mram_addr);  
            break;

        case 8:
            uc_loader.dram_size = (message & 0xFFFF); 
            printf("DRAM Size: %08x\n", uc_loader.dram_size);  
            break;

        case 9:
            uc_loader.dram_dest = (message & 0xFFFF);
            printf("DRAM Destination Address: %08x\n", uc_loader.dram_dest);
            break;
    }

    ucode_steps++;

    if(ucode_steps == 10) {
        ucode_steps = 0;
        resume_mail = true;
        upload_in_progress = false;
        
        //Generate checksum
        crc = GenerateCRC(
            &Mem_RAM[uc_loader.iram_mram_addr & RAM_MASK],
            uc_loader.iram_size);
        printf("Game CRC 32=%08x\n",crc);
    }
}
