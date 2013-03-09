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
}

/**
 * Processes all mail sent to DSP
 * @param message Mail sent to DSP
 */
void UCode_Zelda::ProcessMail(u32 message) {
    if(!sync_in_progress) 
    {
        if(!processing_commands) {
            command_length = (message & 0xFF); //Command lengths are short
            command_count = 0;
            processing_commands = true;
        } else if((processing_commands) && (command_count < command_length)) {
               
            if(command_count == 0) {
                command_type = (message >> 24) & 0x7F;
                printf("Command Type: %08x\n", command_type);
            }
    
            command_list.push_back(message); //Push command to list
            command_count++;

            if(command_count == command_length)
            {
                ProcessCommands(); //Process all commands in the list
            }
        }
    }
}

void UCode_Zelda::Update() { }

/**
 * Processes all commands sent to DSP
 */
void UCode_Zelda::ProcessCommands() {
    switch(command_type)
    {
        //DsetupTable
        case 0x01: 
            printf("Command 0x1: DsetupTable\n");
            printf("Number of voices: %08x\n", (command_list[0] & 0xFFFF));
            printf("Mixing PBs Addr: %08x\n", (command_list[1] & 0x7FFFFFFF));
            printf("ADPCM Coef Table Addr: %08x\n", (command_list[2] & 0x7FFFFFFF));
            printf("AFC Coef Table Addr: %08x\n", (command_list[3] & 0x7FFFFFFF));
            printf("Reverb PBs Addr: %08x\n", (command_list[4] & 0x7FFFFFFF));
            break;

        //DsyncFrame
        case 0x02:
            printf("Command 0x2: DsyncFrame\n");
            printf("Number of buffers: %08x\n", (command_list[0] >> 16) & 0x7F);
            printf("Right Buffer Addr: %08x\n", (command_list[1] & 0x7FFFFFFF));
            printf("Left Buffer Addr: %08x\n", (command_list[2] & 0x7FFFFFFF));
            sync_in_progress = true;
            break;

        //DsetDolbyDelay
        case 0x0D:
            printf("Command 0xD: DsetDolbyDelay");
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

    if(sync_in_progress)
    {
        SendMail(DSP_DONE, 0);
    }

    command_length = 0;
    command_type = 0;
    command_list.clear();
    processing_commands = false;
}









