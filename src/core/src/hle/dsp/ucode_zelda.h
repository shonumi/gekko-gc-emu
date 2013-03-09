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

#ifndef CORE_UCODE_ZELDA_H_
#define CORE_UCODE_ZELDA_H_

#include <vector>
#include "ucode.h"

class UCode_Zelda : public UCode {
public:

    UCode_Zelda(MailManager* mail_mngr);
    virtual ~UCode_Zelda() {};

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
     * Processes all commands sent to DSP
     */
    void ProcessCommands();

private:
    u8 command_length;
    u8 command_count;
    u8 command_type;

    std::vector <u32> command_list;
    bool processing_commands;
    bool sync_in_progress;
};

#endif // CORE_UCODE_ZELDA_H_