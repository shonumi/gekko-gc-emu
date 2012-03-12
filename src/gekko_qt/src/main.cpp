#include <QtGui>
#include "main.hxx"

#include "common.h"
#include "config.h"

#include "core.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
#include "powerpc/cpu_core.h"
#include "hw/hw.h"
#include "video/opengl.h"

#include "disasm.hxx"
#include "gekko_regs.hxx"
#include "image_info.hxx"

#include "version.h"

#define APP_NAME        "gekko"
#define APP_VERSION     "0.31-" VERSION
#define APP_TITLE       APP_NAME " " APP_VERSION
#define COPYRIGHT       "Copyright (C) 2005-2012 Gekko Team"

GMainWindow::GMainWindow()
{
    QString sPath = QDir::currentPath();
    file_browser_model = new QFileSystemModel(this);

    ui.setupUi(this);

    ui.treeView->setModel(file_browser_model);
    file_browser_model->setFilter(QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDot);
    file_browser_model->setRootPath(sPath);
    ui.treeView->setRootIndex(file_browser_model->index(sPath));
	ui.treeView->sortByColumn(0, Qt::AscendingOrder);
    ui.treeView->hideColumn(2); // drive
    ui.treeView->hideColumn(3); // date

	image_info = new GImageInfo(this);
	addDockWidget(Qt::RightDockWidgetArea, image_info);

    GDisAsmView* disasm = new GDisAsmView(this);
    addDockWidget(Qt::TopDockWidgetArea, disasm);

    GGekkoRegsView* gekko_regs = new GGekkoRegsView(this);
    addDockWidget(Qt::LeftDockWidgetArea, gekko_regs);

    // setup connections
    connect(ui.action_Start, SIGNAL(triggered()), this, SLOT(OnStartGame()));
    connect(ui.treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnFileBrowserDoubleClicked(const QModelIndex&)));
	connect(ui.treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnFileBrowserClicked(const QModelIndex&)));
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

void GMainWindow::OnFileBrowserDoubleClicked(const QModelIndex& index)
{
    // TODO: Sometimes, trying to access a directory with lacking read permissions will break stuff (you'll end up in an empty directory with now way to cd back to parent directory)
    if (!file_browser_model->isDir(index))
        return;

    QString new_path = file_browser_model->filePath(index);
    file_browser_model->setRootPath(new_path);
    ui.treeView->setRootIndex(file_browser_model->index(new_path));
}

// TODO: Doesn't belong here! Clean up and separate from GUI code
#if 0
#define PACKRGB555(r, g, b)				(u16)((((r)&0xf8)<<7)|(((g)&0xf8)<<2)|(((b)&0xf8)>>3))
#define PACKRGB565(r, g, b)				(u16)((((r)&0xf8)<<8)|(((g)&0xfc)<<3)|(((b)&0xf8)>>3))

static QPixmap BrowserAddBanner(u8 *banner)
{
    int			width = DVD_BANNER_WIDTH;
    int			height = DVD_BANNER_HEIGHT;
    int			bcount = width * height * 3;
    int			tiles  = (DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT) / 16;    
    u8			*imageA, *ptrA;
    f64			rgb[3];
    u16			*tile = (u16*)banner, *ptrA16;
    u32			r, g, b, a;
    int			row = 0, col = 0, pos;

    imageA = (u8 *)malloc(bcount);

    DWORD backcol = GetSysColor(COLOR_WINDOW);
    rgb[0] = (f64)GetRValue(backcol);
    rgb[1] = (f64)GetGValue(backcol);
    rgb[2] = (f64)GetBValue(backcol);

    // Convert RGB5A3 -> RGBA
    for(int i=0; i<tiles; i++, tile+=16)
    {
        for(int j=0; j<4; j++)
        for(int k=0; k<4; k++)
        {
            u16 p;

            p = tile[j * 4 + k];
            if(p >> 15)
            {
                r = (p & 0x7c00) >> 10;
                g = (p & 0x03e0) >> 5;
                b = (p & 0x001f);
                r = (u8)((r << 3) | (r >> 2));
                g = (u8)((g << 3) | (g >> 2));
                b = (u8)((b << 3) | (b >> 2));
            }
            else
            {
                r = (p & 0x0f00) >> 8;
                g = (p & 0x00f0) >> 4;
                b = (p & 0x000f);
                a = (p & 0x7000) >> 12;

                r = g = b = 0;
            }

            pos = 3 * ((row + j) * width + (col + k));

            ptrA16 = (u16 *)&imageA[pos];
            ptrA   = &imageA[pos];

            p = PACKRGB565(r, g, b);

            *ptrA++ = (u8)r;
            *ptrA++ = (u8)g;
            *ptrA++ = (u8)b;
        }

        col += 4;
        if(col == DVD_BANNER_WIDTH)
        {
            col = 0;
            row += 4;
        }
    }

    QPixmap pm_banner = QPixmap::fromImage(QImage(imageA, 96, 32, QImage::Format_RGB888));
    free(imageA);
    return pm_banner;
}
#endif

void GMainWindow::OnFileBrowserClicked(const QModelIndex& index)
{
// TODO: Make ReadGCMInfo cross platform...
#if 0
// TODO: Causes instability?
	if (file_browser_model->isDir(index))
		return;

	unsigned long size;
	u8 banner[0x1960];
	u8 header[0x440];
	if (dvd::ReadGCMInfo(file_browser_model->filePath(index).toLatin1().data(), &size, (void*)banner, (void*)header) != E_OK)
		return;

	QPixmap pm_banner = BrowserAddBanner(banner);
	image_info->SetBanner(pm_banner);
#endif
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
