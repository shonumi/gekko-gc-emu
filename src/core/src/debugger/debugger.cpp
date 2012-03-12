#include <vector>
#include <set>
#include "core.h"
#include "debugger.h"

// TODO: Init / Uninit these 
static std::set<u32> breakpoints;
static std::vector<Debugger::CPUSteppedCallback> cpu_step_observers;

bool Debugger::GetCallstack(std::vector<CallstackEntry>& out)
{
    if (core::g_state != core::SYS_HALTED && core::g_state != core::SYS_DEBUG)
        return false;

    // TODO: Implement!
    return false;
}

void Debugger::SetBreakpoint(u32 addr)
{
    breakpoints.insert(addr);
}

void Debugger::UnsetBreakpoint(u32 addr)
{
    breakpoints.erase(addr);
}

void Debugger::UnsetAllBreakpoints()
{
    breakpoints.clear();
}

// TODO: Make CPU engines break at breakpoints
bool Debugger::IsBreakpoint(u32 addr)
{
    return breakpoints.find(addr) != breakpoints.end();
}

// TODO: Call this whenever the CPU engines processed one or more instructions
void Debugger::CPUStepped()
{
    std::vector<CPUSteppedCallback>::iterator obs_it = cpu_step_observers.begin();
    for (; obs_it != cpu_step_observers.end(); ++obs_it)
        (*obs_it)();
}

void Debugger::RegisterCPUStepCallback(Debugger::CPUSteppedCallback func)
{
    cpu_step_observers.push_back(func);
}

void Debugger::UnregisterCPUStepCallback(Debugger::CPUSteppedCallback func)
{
    std::remove(cpu_step_observers.begin(), cpu_step_observers.end(), func);
}
