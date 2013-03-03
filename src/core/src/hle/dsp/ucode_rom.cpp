/*!
 * Copyright (C) 2005-2013 Gekko Emulator
 *
 * @file    ucode_rom.cpp
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2013-02-19
 * @brief   Ucode ROM
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

#include "ucode_rom.h"
#include "crc.h"
#include "memory.h"

//UCode_ROM Constructor
UCode_ROM::UCode_ROM() {    
    mail_man.Clear();
    //mail_man.PushMail(0x8071FEED);
}

/**
 * Processes all mail sent to DSP
 * @param message Mail sent to DSP
 */
void UCode_ROM::ProcessMail(u32 message) {
    if(next_parameter == 0) {

        if((message & 0xFFFF0000) != 0x80F30000) {
            message = 0xFEEE0000 | (message & 0xFFFF);
            mail_man.PushMail(message);
        } else {
            next_parameter = message;
        }

    } else {
            switch(next_parameter) {
                
                case 0x80F3A001:
                    current_ucode.ram_addr = message; 
                    printf("UCode ROM RAM Address: %08x\n", current_ucode.ram_addr);
                    break;

                case 0x80F3A002:
                    current_ucode.length = message & 0xFFFF; 
                    printf("UCode ROM Length: %08x\n", current_ucode.length);
                    break;

                case 0x80F3C002:
                    current_ucode.imem_addr = message & 0xFFFF; 
                    printf("UCode ROM IMEM Address: %08x\n", current_ucode.imem_addr);
                    break;
 
                case 0x80f3b002:
                    current_ucode.dmem_length = message & 0xFFFF; 
                    printf("UCode ROM DMEM Length: %08x\n", current_ucode.dmem_length);
                    break;

                case 0x80F3D001:
                    current_ucode.start_pc = message & 0xFFFF;
                    printf("UCode ROM Start PC: %08x\n", current_ucode.start_pc);
                    BootUCode();
                    break;

                default: break;
            }

            next_parameter = 0;
    }
}       
       
void UCode_ROM::Update() { }

/**
 * Sets ucode up to boot
 * @param message Mail sent to DSP for booting
 */
void UCode_ROM::BootUCode() {
    crc = GenerateCRC(&Mem_RAM[current_ucode.dmem_length & RAM_MASK],
				    current_ucode.length);
    printf("UCode ROM BootUCode Generates CRC: %08x\n",crc);
    upload_in_progress = false;
}
