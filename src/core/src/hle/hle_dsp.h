/*!
 * Copyright (C) 2005-2013 Gekko Emulator
 *
 * @file    hle_dsp.h
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2013-2-15
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
 
#ifndef CORE_DSP_HLE_H_
#define CORE_DSP_HLE_H_

#include "hle/dsp/ucode.h" 

class DSPHLE {
public:

    DSPHLE() {};
    ~DSPHLE() {};

    /**
     * @brief Writes upper word of a 32-bit mail message
     * @param cpu_mbox If true write to CPU's mailbox, if false write to DSP's mailbox
     * @param message The mail message itself
     */
    void DSP_WriteMailboxHi(bool cpu_mbox, u16 message);

    /**
     * @brief Writes lower word of a 32-bit mail message
     * @param cpu_mbox If true write to CPU's mailbox, if false write to DSP's mailbox
     * @param message The mail message itself
     */
    void DSP_WriteMailboxLo(bool cpu_mbox, u16 message);

    /**
     * @brief Reads upper word of a 32-bit mail message from mailbox
     * @param cpu_mbox If true read from CPU's mailbox, if false read from DSP's mailbox
     * @returns Unsigned word of mail message
     */
    u16 DSP_ReadMailboxHi(bool cpu_mbox);

    /**
     * @brief Reads lower word of a 32-bit mail message from mailbox
     * @param cpu_mbox If true read from CPU's mailbox, if false read from DSP's mailbox
     * @returns Unsigned word of mail message
     */
    u16 DSP_ReadMailboxLo(bool cpu_mbox);

   /**
    * @brief Initializes the DSP - sets various values and such
    * @returns If initialized correctly, return true, false if otherwise
    */
    bool Init();

   /**
    * @brief Gets the CRC from the UCode
    * @returns CRC of the current UCode, 0 if UCode pointer is null
    */
    u32 GetCRC();

    /**
     * @brief Gets the upload status of the UCode
     * @returns Boolean value of current UCode upload_in_progress
     */
    bool UploadStatus();

    /**
     * @brief Set/Switch current UCode
     * @param crc CRC used to generate new UCode
     */
    void SetUCode(u32 crc);

    UCode* game_ucode; //Not quite ready to make these private yet

    u32 mbox_cpu_dsp;
    u32 mbox_dsp_cpu;

    private:

    MailManager mail_man;

    DISALLOW_COPY_AND_ASSIGN(DSPHLE);
};

#endif // CORE_DSP_HLE_H_