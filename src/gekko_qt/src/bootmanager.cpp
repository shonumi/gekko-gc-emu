#include "common.h"
#include "video/opengl.h"
#include "bootmanager.hxx"

#include "config.h"

#include "core.h"
#include "memory.h"
#include "debugger/debugger.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
#include "powerpc/cpu_core.h"
#include "hw/hw.h"
#include "debugger/debugger.h"

#include "version.h"

#define APP_NAME        "gekko"
#define APP_VERSION     "0.31-" VERSION
#define APP_TITLE       APP_NAME " " APP_VERSION
#define COPYRIGHT       "Copyright (C) 2005-2012 Gekko Team"

EmuThread::EmuThread(GRenderWindow* render_window) : exec_cpu_step(false), cpu_running(true), render_window(render_window)
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

    render_window->makeCurrent();
    OPENGL_SetWindow(render_window);
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
                    if (!cpu_running)
                    {
                        emit CPUStepped();
                        exec_cpu_step = false;
                        cpu->step = true;
                        while (!exec_cpu_step && !cpu_running);
                    }
                    cpu->execStep();
                    cpu->step = false;
                }
#endif
            }
        } else if (core::SYS_HALTED == core::g_state) {
            core::Stop();
        }
    }
    OPENGL_Kill();
    render_window->doneCurrent();
    core::Kill();
}

void EmuThread::Stop()
{
    if (!isRunning())
    {
        LOG_INFO(TMASTER, "EmuThread::Stop called while emu thread wasn't running, returning...");
        return;
    }

    core::g_state = core::SYS_DIE;

    wait(1000);
    if (isRunning())
    {
        LOG_WARNING(TMASTER, "EmuThread still running, terminating...");
        terminate();
        wait(1000);
        if (isRunning())
            LOG_WARNING(TMASTER, "EmuThread STILL running, something is wrong here...");
    }
    LOG_INFO(TMASTER, "EmuThread stopped");
}


GRenderWindow::GRenderWindow(QWidget* parent): QGLWidget(parent), emu_thread(this)
{
    setAutoBufferSwap(false);
    resize(640, 480); // TODO...
}

GRenderWindow::~GRenderWindow()
{
    emu_thread.Stop();
}

void GRenderWindow::paintEvent(QPaintEvent* )
{
    // Overloaded to prevent the GUI thread from stealing GL context
    // Handled in EmuThread instead

}

void GRenderWindow::resizeEvent(QResizeEvent* )
{
    // Overloaded to prevent the GUI thread from stealing GL context
    // Handled in EmuThread instead
}

void GRenderWindow::closeEvent(QCloseEvent* event)
{
    emu_thread.Stop();
    QGLWidget::closeEvent(event);
}

EmuThread& GRenderWindow::GetEmuThread()
{
    return emu_thread;
}

void GRenderWindow::SetTitle(const char* title)
{
    setWindowTitle(title);
}

void GRenderWindow::SwapBuffers()
{
    swapBuffers();
}
