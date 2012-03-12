/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    debugger.h
 * \author  neobrain <NeoBrainX@gmail.com>
 * \date    2012-03-12
 * \brief   Interface for accessing debug info from debugger UIs
 *
 * \section LICENSE
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

#ifndef CORE_DEBUGGER_H_
#define CORE_DEBUGGER_H_

#include <vector>
#include "common.h"

namespace Debugger
{

struct CallstackEntry
{
    std::string name;
    u32 addr;
};
typedef std::vector<CallstackEntry> Callstack;
typedef Callstack::iterator CallstackIterator;

/**
 * Get call stack of current PC. Needs to be called when CPU is paused.
 *
 * @param out vector of call stack entries (first element = topmost entry)
 * @return true on success; false on failure (e.g. function called when CPU was paused)
 * @warning not thread safe!
 */
bool GetCallstack(Callstack& out);

/**
 * Sets a CPU breakpoint.
 *
 * @param addr CPU instruction address to break at.
 */
void SetBreakpoint(u32 addr);

/**
 * Unsets a CPU breakpoint
 *
 * @param addr breakpoint address
 */
void UnsetBreakpoint(u32 addr);

/**
 * Unsets all CPU breakpoints
 */
void UnsetAllBreakpoints();

/**
 * Check if the CPU instruction at the given address is a breakpoint
 *
 * @param addr CPU instruction address to check
 * @return true if addr is a breakpoint
 */
bool IsBreakpoint(u32 addr);

/**
 * Called by the CPU whenever an instruction has been executed.
 * Calls the callback functions registered by RegisterCPUStepCallback.
 */
void CPUStepped();

typedef void (*CPUSteppedCallback)(void*);

/**
 * Adds a callback function to be called whenever CPUStepped() is called.
 *
 * @param func pointer to callback function
 * @param data optional data pointer to be passed whenever the function gets called (e.g. this-pointer for class methods)
 * @warning Don't forget unregistering the function via UnregisterCPUStepCallback
 */
void RegisterCPUStepCallback(CPUSteppedCallback func, void* data);

/**
 * Removes a callback function from the CPUStepped() observer list
 */
void UnregisterCPUStepCallback(CPUSteppedCallback func);


} // namespace Debugger

#endif // CORE_DEBUGGER_H_
