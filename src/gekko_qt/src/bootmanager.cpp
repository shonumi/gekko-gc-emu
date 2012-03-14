#include "bootmanager.hxx"

#include "common.h"
#include "config.h"

#include "core.h"
#include "memory.h"
#include "debugger/debugger.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
#include "powerpc/cpu_core.h"
#include "hw/hw.h"
#include "video/opengl.h"

#include "version.h"

#define APP_NAME        "gekko"
#define APP_VERSION     "0.31-" VERSION
#define APP_TITLE       APP_NAME " " APP_VERSION
#define COPYRIGHT       "Copyright (C) 2005-2012 Gekko Team"

static EmuThread* emu_thread = NULL;

void EmuThread::Init()
{
    emu_thread = new EmuThread;
}

void EmuThread::Shutdown()
{
    delete emu_thread;
}

EmuThread* EmuThread::GetInstance()
{
    return emu_thread;
}

EmuThread::EmuThread() : exec_cpu_step(false), cpu_running(true)
{
}

void EmuThread::SetFilename(const char* filename)
{
    strcpy(this->filename, filename);
}

void EmuThread::run()
{
    u32 tight_loop;
    LOG_NOTICE(TMASTER, APP_NAME " starting...\n");

    OPENGL_SetTitle(APP_TITLE); // TODO(ShizZy): Find a better place for this

    if (E_OK != core::Init()) {
        LOG_ERROR(TMASTER, "core initialization failed, exiting...");
        core::Kill();
        exit(1);
    }

#ifdef USE_INLINE_ASM
    // If using asm, see if this computer can process
    LOG_INFO(TMASTER, "compiled with inline assembly... ");
    if (E_OK == common::IsSSE2Supported()) {
        LOG_APPEND(LINFO, TMASTER, "SSE2 found\n");
    } else {
        LOG_ERROR(TMASTER, "compiled with inline assembly, but your CPU architecture does not "
            "support SSE2, exiting...");
        core::Kill();
        exit(1);
    }
#endif

    // Load a game or die...
    if (E_OK == dvd::LoadBootableFile(filename)) {
        if (common::g_config->enable_auto_boot()) {
            core::Start();
        } else {
            LOG_ERROR(TMASTER, "Autoboot required in no-GUI mode... Exiting!\n");
        }
    } else {
        LOG_ERROR(TMASTER, "Failed to load a bootable file... Exiting!\n");
        exit(E_ERR);
    }

    while(core::SYS_DIE != core::g_state) {
        if (core::SYS_RUNNING == core::g_state) {
            if(!cpu->is_on) {
            cpu->Start(); // Initialize and start CPU.
            } else {
#ifdef USE_INLINE_ASM
                _asm {
                    mov tight_loop, 10000

                ContinueCPULoop:
                    mov ecx, cpu
                    cmp GekkoCPU::pause, 0
                    jne CPULoopDone
                    mov edx, [ecx] //call cpu->ExecuteInstruction
                    call [edx]
                    mov ecx, cpu
                    mov edx, [ecx]	//call cpu->ExecuteInstruction
                    call [edx]
                    mov ecx, cpu
                    mov edx, [ecx]	//call cpu->ExecuteInstruction
                    call [edx]
                    mov ecx, cpu
                    mov edx, [ecx]	//call cpu->ExecuteInstruction
                    call [edx]
                    mov ecx, cpu
                    mov edx, [ecx]	//call cpu->ExecuteInstruction
                    call [edx]
                    cmp core::g_state, 2 // 2 is core::SYS_RUNNING
                    jne CPULoopDone
                    sub tight_loop, 1
                    jnz ContinueCPULoop
                CPULoopDone:
                };
#else
                for(tight_loop = 0; tight_loop < 10000; ++tight_loop) {
                    // TODO: if debugger enabled...
                    while (!exec_cpu_step && !cpu_running);
                    cpu->execStep();

                    if (!cpu_running)
                    {
                        emit CPUStepped();
                        exec_cpu_step = false;
                    }
                }
#endif
            }
        } else if (core::SYS_HALTED == core::g_state) {
            core::Stop();
        }
    }
    core::Kill();
}
