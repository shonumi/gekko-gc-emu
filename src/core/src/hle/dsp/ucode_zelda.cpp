/*!
 * Copyright (C) 2005-2013 Gekko Emulator
 *
 * @file    ucode_zelda.h
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2013-03-07
 * @brief   Zelda UCode
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

#include "ucode_zelda.h"

/**
 * UCode_Zelda Constructor
 */
UCode_Zelda::UCode_Zelda(MailManager* mail_mngr) {
    mail_man = mail_mngr;
    mail_man->Clear();

    //Initialize Zelda UCode Handshake
    SendMail(DSP_INIT, 1);
    SendMail(0xF3551111,0);
    printf("Zelda UCode: Sending Handshake\n");

    command_length = 0;
    command_count = 0;
    processing_commands = false;
    sync_in_progress = false;

    max_voices = 0;
    num_voices = 0;
    current_buffer = 0;
    num_buffers = 0;
}

/**
 * Processes all mail sent to DSP
 * @param message Mail sent to DSP
 */
void UCode_Zelda::ProcessMail(u32 message) {

    //Check for sync mails or start grabbing commands
    if(!sync_in_progress) 
    {
	if(message == 0) { //Check for sync mails first
            sync_in_progress = true;
        } else if(message == 0xCDD10003) {
            printf("Sync Ending Mail Received\n");
        } else if(!processing_commands) { //Check for commands next
            command_length = (message & 0xFF);
            command_count = 0;
            processing_commands = true;
        } else if((processing_commands) && (command_count < command_length)) {
               
            if(command_count == 0) {
                command_type = (message >> 24) & 0x7F;
                printf("Command Type: %08x\n", command_type);
            }
    
            command_list.push_back(message); //Push command to list
            command_count++;

            if(command_count == command_length) {
                ProcessCommands(); //Process all commands in the list
            }
        }
    }

    //Sync mail handling
    else {
        sync_in_progress = false;
        u32 n = (message >> 16) & 0xF;
        max_voices = (n + 1) << 4;

        if(max_voices >= num_voices) {
            current_buffer++;
            SendMail(DSP_SYNC, 1);
            SendMail(0xF355FF00 | current_buffer, 0);
 
            if(current_buffer == num_buffers) {
                SendMail(DSP_FRAME_END, 0);
                current_buffer = 0;
            }
        }
    }
}

/**
 * Updates UCode
 */
void UCode_Zelda::Update() { 
    //Request PI Interrupt after sending DSP_FRAME_END
    if(mail_man->ReadNextMail() == DSP_FRAME_END) { 
        REGDSP16(DSP_CSR) |= DSP_CSR_DSPINT; 
        dspCSRDSPInt = DSP_CSR_DSPINT; 
    }
}

/**
 * Processes all commands sent to DSP
 */
void UCode_Zelda::ProcessCommands() {
    switch(command_type) {

        //DsetupTable
        case 0x01: 
            printf("Command 0x1: DsetupTable\n");
            printf("Number of voices: %08x\n", (command_list[0] & 0xFFFF));
            num_voices = (command_list[0] & 0xFFFF);
            printf("Mixing PBs Addr: %08x\n", (command_list[1] & 0x7FFFFFFF));
            printf("ADPCM Coef Table Addr: %08x\n", (command_list[2] & 0x7FFFFFFF));
            printf("AFC Coef Table Addr: %08x\n", (command_list[3] & 0x7FFFFFFF));
            printf("Reverb PBs Addr: %08x\n", (command_list[4] & 0x7FFFFFFF));
            break;

        //DsyncFrame
        case 0x02:
            printf("Command 0x2: DsyncFrame\n");
            printf("Number of buffers: %08x\n", (command_list[0] >> 16) & 0x7F);
            num_buffers = (command_list[0] >> 16) & 0x7F;
            current_buffer = 0;
            printf("Right Buffer Addr: %08x\n", (command_list[1] & 0x7FFFFFFF));
            printf("Left Buffer Addr: %08x\n", (command_list[2] & 0x7FFFFFFF));
            break;

        //DsetDolbyDelay
        case 0x0D:
            printf("Command 0xD: DsetDolbyDelay\n");
            break;

        //DsetDMABaseAddr
        case 0x0E:
            printf("Command 0xE: DsetDMABaseAddr\n");
            break;

        default:
            printf("Unknown Zelda ucode %08x\n", command_type);      
    }

    SendMail(DSP_SYNC, 1);
    SendMail(0xF3550000 | (command_list[0] >>16), 0);

    command_length = 0;
    command_type = 0;
    command_list.clear();
    processing_commands = false;
}
