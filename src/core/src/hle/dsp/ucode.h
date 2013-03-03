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

#ifndef CORE_UCODE_H_
#define CORE_UCODE_H_

#include "mail_manager.h"

#define UCODE_ROM            0x0000000

class UCode {
public:

    UCode();
    virtual ~UCode() {};

    /**
     * Sets ucode up to boot
     * @param message Mail sent to DSP for booting
     */
    void PrepareBootUCode(u32 message);

    /**
     * Processes all mail sent to DSP
     * @param message Mail sent to DSP
     */
    virtual void ProcessMail(u32 message) = 0;

    /**
     * Updates UCode
     */
    virtual void Update() = 0;

    /**
     * Sends mail from DSP to CPU
     * @param message Mail going to CPU
     * @param interrupt_req Requests interrupt from DSP if 1
     */
    void SendMail(u32 message, int interrupt_req);

    /**
     * Determines if we need to send a resume mail to CPU
     * @returns True if resume mail flag is set, false if not
     */
    bool ResumeMail();

    enum DSPCodes
    {
            DSP_INIT = 0xDCD10000,
            DSP_RESUME = 0xDCD10001,
            DSP_YIELD = 0xDCD10002,
            DSP_DONE = 0xDCD10003,
            DSP_SYNC = 0xDCD10004,
            DSP_FRAME_END = 0xDCD10005,
     };

    MailManager mail_man;

    bool resume_mail;
    bool upload_in_progress;
    u32 crc;

protected:

    struct UCodeLoader {
        u32 mram_dest_addr;
        u16 mram_size;
        u16 mram_dram_addr;
        u32 iram_mram_addr;
        u16 iram_size;
        u16 iram_dest;
        u16 iram_startpc;
        u32 dram_mram_addr;
        u16 dram_size;
        u16 dram_dest;
    };

    UCodeLoader uc_loader;
    int ucode_steps;
};

/**
 * Generates a new UCode from CRC checksum
 * @param crc Checksum to use
 * @returns Pointer to new UCode
 */
UCode* GenerateUCode(u32 crc);

#endif // CORE_UCODE_H_