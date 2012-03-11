#include <QtGui>
#include "ui_main.h"
#include "main.hxx"

#include "common.h"
#include "config.h"

#include "core.h"
#include "dvd/loader.h"
#include "powerpc/cpu_core.h"
#include "hw/hw.h"
#include "video/opengl.h"

#include "disasm.hxx"
#include "gekko_regs.hxx"

#include "version.h"

#define APP_NAME        "gekko"
#define APP_VERSION     "0.31-" VERSION
#define APP_TITLE       APP_NAME " " APP_VERSION
#define COPYRIGHT       "Copyright (C) 2005-2012 Gekko Team"

GMainWindow::GMainWindow()
{
    QString sPath = "C:/";
    QFileSystemModel* dirmodel = new QFileSystemModel(this);
    dirmodel->setRootPath(sPath);

    Ui::MainWindow main_window_ui;
    main_window_ui.setupUi(this);

//    main_window_ui.treeView->setModel(dirmodel);
    main_window_ui.treeView->hideColumn(2); // drive
    main_window_ui.treeView->hideColumn(3); // date

    GDisAsmView* disasm = new GDisAsmView(this);
    addDockWidget(Qt::TopDockWidgetArea, disasm);

    GGekkoRegsView* gekko_regs = new GGekkoRegsView(this);
    addDockWidget(Qt::LeftDockWidgetArea, gekko_regs);

    // setup connections
    connect(main_window_ui.action_Start, SIGNAL(triggered()), this, SLOT(OnStartGame()));
}

void GMainWindow::OnStartGame()
{
    u32 tight_loop;
    LOG_NOTICE(TMASTER, APP_NAME " starting...\n");

    char program_dir[MAX_PATH];
    _getcwd(program_dir, MAX_PATH-1);
    size_t cwd_len = strlen(program_dir);
    program_dir[cwd_len] = '/';
    program_dir[cwd_len+1] = '\0';

    OPENGL_SetTitle(APP_TITLE); // TODO(ShizZy): Find a better place for this
    common::ConfigManager config_manager;
    config_manager.set_program_dir(program_dir, MAX_PATH);
    config_manager.ReloadConfig(NULL);
    core::SetConfigManager(&config_manager);

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
    if (E_OK == dvd::LoadBootableFile(common::g_config->default_boot_file())) {
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
                    cpu->execStep();
                }
#endif
            }
        } else if (core::SYS_HALTED == core::g_state) {
            core::Stop();
        }
    }
    core::Kill();
//        return E_OK;
}

#ifdef main
#undef main
#endif

int __cdecl main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GMainWindow main_window;

    main_window.show();
    return app.exec();
}
