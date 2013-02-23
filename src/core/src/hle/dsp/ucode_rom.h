/*!
 * Copyright (C) 2005-2013 Gekko Emulator
 *
 * @file    ucode_rom.h
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

#ifndef CORE_UCODE_ROM_H_
#define CORE_UCODE_ROM_H_

#include "ucode.h"

class UCode_ROM : public UCode {
public:

    UCode_ROM();
    virtual ~UCode_ROM() {};

    /**
     * Processes all mail sent to DSP
     * @param message Mail sent to DSP
     */
    void ProcessMail(u32 message);

    /**
     * Updates UCode
     */
    void Update();

    /**
     * Updates UCode
     */
    void BootUCode();

private:

    struct UCodeLoader {
        u32 ram_addr;
        u32 length;
        u32 imem_addr;
        u32 dmem_length;
        u32 start_pc;
    };

    UCodeLoader current_ucode;
    u32 boot_steps;
    u32 next_parameter;
};

#endif // CORE_UCODE_ROM_H_