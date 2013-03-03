/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    hle_dsp.cpp
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2013-16-02
 * @brief   Class for High Level DSP Emulation
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

#include "hle_dsp.h"

/**
 * Writes upper word of a 32-bit mail message
 * @param cpu_mbox If true write to CPU's mailbox, if false write to DSP's mailbox
 * @param message The mail message itself
 */
void DSPHLE::DSP_WriteMailboxHi(bool cpu_mbox, u16 message) {
    if(cpu_mbox) {
        mbox_cpu_dsp = (mbox_cpu_dsp & 0xFFFF) | (message << 16);
    } else {
        printf("DSPHLE::Warning, CPU attempted to write to the DSP's mailbox\n");
    }
}

/**
 * Writes lower word of a 32-bit mail message
 * @param cpu_mbox If true write to CPU's mailbox, if false write to DSP's mailbox
 * @param message The mail message itself
 */
void DSPHLE::DSP_WriteMailboxLo(bool cpu_mbox, u16 message) {
    if(cpu_mbox) {
        mbox_cpu_dsp = (mbox_cpu_dsp & 0xFFFF0000) | message;
        game_ucode->ProcessMail(mbox_cpu_dsp);
	mbox_cpu_dsp &= 0x7FFFFFFF;
    } else {
        printf("DSPHLE::Warning, CPU attempted to write to the DSP's mailbox\n");
    }
}

/**
 * Reads upper word of a 32-bit mail message from mailbox
 * @param cpu_mbox If true read from CPU's mailbox, if false read from DSP's mailbox
 * @returns Unsigned word of mail message
 */
u16 DSPHLE::DSP_ReadMailboxHi(bool cpu_mbox) {
    if(cpu_mbox) {
        return (mbox_cpu_dsp >> 16) & 0xFFFF;
    } else {
        return mail_man.ReadMailboxHi();
    }
}

/**
 * Reads lower word of a 32-bit mail message from mailbox
 * @param cpu_mbox If true read from CPU's mailbox, if false read from DSP's mailbox
 * @returns Unsigned word of mail message
 */
u16 DSPHLE::DSP_ReadMailboxLo(bool cpu_mbox) {
    if(cpu_mbox) {
        return (mbox_cpu_dsp & 0xFFFF);
    } else {
        return mail_man.ReadMailboxLo();
    }
}

/**
 * @brief Initializes the DSP - sets various values and such
 * @returns If initialized correctly, return true, false if otherwise
 */
bool DSPHLE::Init() {
    game_ucode = NULL;
    mbox_cpu_dsp = 0;
    mbox_dsp_cpu = 0;
    mail_man.Clear();
}

/**
 * @brief Gets the CRC from the UCode
 * @returns CRC of the current UCode, 0 if UCode pointer is null
 */
u32 DSPHLE::GetCRC() {
    if(game_ucode == NULL) {
        return 0;
    } else {
        return game_ucode->crc;
    }
}

/**
 * @brief Gets the upload status of the UCode
 * @returns Boolean value of current UCode upload_in_progress
 */
bool DSPHLE::UploadStatus() {
    if(game_ucode == NULL) {
        return true; //If UCode is null, treat it so that we don't try to boot into another
    } else {
        return game_ucode->upload_in_progress;
    }
}

/**
 * @brief Set/Switch current UCode
 * @param crc CRC used to generate new UCode
 */
void DSPHLE::SetUCode(u32 crc) {
    delete game_ucode;
    game_ucode = GenerateUCode(crc);
}

