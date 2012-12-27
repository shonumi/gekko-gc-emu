/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    mail_manager.h
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2012-12-25
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

#include "mail_manager.h"

void MailManager::PushMail(u32 message) {
    message_queue.push(message);
}

u16 MailManager::ReadMailboxHi() {
    // Make sure mail exists... 
    if(!message_queue.empty())
    {
        u16 result = (message_queue.front() >> 16) & 0xFFFF;
        return result;
    }

    // ...Else return 0
    return 0;
}

u16 MailManager::ReadMailboxLo() {
    // Make sure mail exists...
    if(!message_queue.empty())
    {
        u16 result = message_queue.front() & 0xFFFF;
        message_queue.pop(); //If we're reading low part, we're done with this mail
        return result;
    }

    // ...Else return 0
    return 0;
}

u32 MailManager::ReadNextMail() {
    return message_queue.front();
}

bool MailManager::Empty() {
    return message_queue.empty();
}

MailManager m_mails;
