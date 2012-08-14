#include <QHBoxLayout>
#include <QKeyEvent>

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

#include "keyboard_input/keyboard_input.h"

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

    if (E_OK != core::Init(render_window)) {
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
            core::Start(render_window);
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
                        while (!exec_cpu_step && !cpu_running && core::SYS_DIE != core::g_state);
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


/*
 * This class overrides paintEvent and resizeEvent to prevent the GUI thread from stealing GL context.
 * The corresponding functionality is handled in EmuThread instead
 */
class GGLWidgetInternal : public QGLWidget
{
public:
    GGLWidgetInternal(GRenderWindow* parent) : QGLWidget(parent)
    {
        setAutoBufferSwap(false);
        doneCurrent();
    }

    void paintEvent(QPaintEvent* ev)
    {
        // Apparently, Windows doesn't display anything if we don't call this here.
        // TODO: Breaks linux though because we aren't calling doneCurrent() ... -.-
//        makeCurrent();
    }
    void resizeEvent(QResizeEvent*) {}
};


EmuThread& GRenderWindow::GetEmuThread()
{
    return emu_thread;
}

GRenderWindow::GRenderWindow(QWidget* parent) : QWidget(parent), emu_thread(this)
{
    // TODO: Enable layout code once core supports a different EmuWindow size than 640x480
    child = new GGLWidgetInternal(this);
    // TODO: One of these flags might be interesting: WA_OpaquePaintEvent, WA_NoBackground, WA_DontShowOnScreen, WA_DeleteOnClose
//    QBoxLayout* layout = new QHBoxLayout(this);
    resize(640, 480);
    child->resize(640, 480);
    child->show();
//    layout->addWidget(child);
//    layout->setMargin(0);
//    setLayout(layout);
    BackupGeometry();
}

GRenderWindow::~GRenderWindow()
{
    emu_thread.Stop();
}

void GRenderWindow::SetTitle(const char* title)
{
	// TODO: Currently called from GPU thread but we should call this from the GUI thread (setWindowTitle is not thread-safe)
//    setWindowTitle(title);
}

void GRenderWindow::SwapBuffers()
{
    child->makeCurrent(); // TODO: Not necessary?
    child->swapBuffers();
}

void GRenderWindow::closeEvent(QCloseEvent* event)
{
    emu_thread.Stop();
    QWidget::closeEvent(event);
}

void GRenderWindow::MakeCurrent()
{
    child->makeCurrent();
}

void GRenderWindow::DoneCurrent()
{
    child->doneCurrent();
}

void GRenderWindow::BackupGeometry()
{
    geometry = ((QGLWidget*)this)->saveGeometry();
}

void GRenderWindow::RestoreGeometry()
{
    // We don't want to back up the geometry here (obviously)
    QWidget::restoreGeometry(geometry);
}

void GRenderWindow::restoreGeometry(const QByteArray& geometry)
{
    // Make sure users of this class don't need to deal with backing up the geometry themselves
    QWidget::restoreGeometry(geometry);
    BackupGeometry();
}

QByteArray GRenderWindow::saveGeometry()
{
    // If we are a top-level widget, store the current geometry
    // otherwise, store the last backup
    if (parent() == NULL)
        return ((QGLWidget*)this)->saveGeometry();
    else
        return geometry;
}

void GRenderWindow::keyPressEvent(QKeyEvent* event)
{
    bool key_processed = false;
    for (unsigned int channel = 0; channel < 4 && GetControllerInterface(); ++channel)
        if (GetControllerInterface()->SetControllerStatus(channel, event->key(), input_common::GCController::PRESSED))
            key_processed = true;

    if (!key_processed)
        QWidget::keyPressEvent(event);
}

void GRenderWindow::keyReleaseEvent(QKeyEvent* event)
{
    bool key_processed = false;
    for (unsigned int channel = 0; channel < 4 && GetControllerInterface(); ++channel)
        if (GetControllerInterface()->SetControllerStatus(channel, event->key(), input_common::GCController::RELEASED))
            key_processed = true;

    if (!key_processed)
        QWidget::keyPressEvent(event);
}

