/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    mail_manager.h
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2012-12-14
 * @brief   Class for handling DSP mails
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

#ifndef CORE_MAIL_MANAGER_H_
#define CORE_MAIL_MANAGER_H_

#include <queue>

#include "common.h"

class MailManager {
public:

    MailManager() {};
    ~MailManager() {};

    /**
     * Puts message into queue
     * @param message Mail sent to DSP
     */
    void PushMail(u32 message);

    /**
     * Checks if message queue is empty
     * @returns True if message queue is empty, false if not
     */
    bool Empty();    

    /**
     * Reads upper word of a 32-bit mail message
     * @returns Unsigned word of mail message
     */
    u16 ReadMailboxHi();

    /**
     * Reads lower word of a 32-bit mail message
     * @returns Unsigned word of mail message
     */
    u16 ReadMailboxLo();

    /**
     * Reads next mail message in queue
     * @returns Unsigned 32-bit mail message from queue
     */
    u32 ReadNextMail();

private:
    std::queue <u32> message_queue;

    DISALLOW_COPY_AND_ASSIGN(MailManager);
};

// NOTE: Very bad form. Using now just to test functionality. 
// Move instance to UCode class when finished (as non-globlal too)
extern MailManager m_mails; 

#endif // CORE_MAIL_MANAGER_H_ 
