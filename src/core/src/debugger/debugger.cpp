#include <vector>
#include <set>
#include "core.h"
#include "debugger.h"

// TODO: Init / Uninit these 
static std::set<u32> breakpoints;

// pair of function pointer and argument
typedef std::pair<Debugger::CPUSteppedCallback, void*> CPUStepCallbackObject;

static std::vector<CPUStepCallbackObject> cpu_step_observers;

bool Debugger::GetCallstack(Callstack& out)
{
//    if (core::g_state != core::SYS_HALTED && core::g_state != core::SYS_DEBUG)
 //       return false;


    CallstackEntry entry;
    entry.name = "some toplevel func";
    entry.addr = 0x80004100;
    out.push_back(entry);
    entry.name = "some other func";
    entry.addr = 0x80004114;
    out.push_back(entry);

    return true;
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
    std::vector<CPUStepCallbackObject>::iterator obs_it = cpu_step_observers.begin();
    for (; obs_it != cpu_step_observers.end(); ++obs_it)
        (obs_it->first)(obs_it->second);
}

void Debugger::RegisterCPUStepCallback(Debugger::CPUSteppedCallback func, void* data)
{
    cpu_step_observers.push_back(CPUStepCallbackObject(func, data));
}

void Debugger::UnregisterCPUStepCallback(Debugger::CPUSteppedCallback func)
{
    std::vector<CPUStepCallbackObject>::iterator obs_it = cpu_step_observers.begin();
    for (; obs_it != cpu_step_observers.end(); ++obs_it)
        if (obs_it->first == func)
        {
            cpu_step_observers.erase(obs_it);
            break;
        }
}
