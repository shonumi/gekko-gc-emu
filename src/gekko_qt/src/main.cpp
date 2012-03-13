#include <QtGui>
#include "qhexedit.h"
#include "main.hxx"

#include "common.h"
#include "config.h"

#include "callstack.hxx"
#include "disasm.hxx"
#include "gekko_regs.hxx"
#include "image_info.hxx"
#include "ramview.hxx"

#include "bootmanager.h"

#include "core.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
#include "version.h"

GMainWindow::GMainWindow() : emu_thread(NULL)
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

    GCallstackView* callstack = new GCallstackView(this);
    addDockWidget(Qt::BottomDockWidgetArea, callstack);

    QDockWidget* dock_ramedit = new QDockWidget(this);
    dock_ramedit->setObjectName("RamViewer");
    ram_edit = new GRamView(dock_ramedit);
    dock_ramedit->setWidget(ram_edit);
    dock_ramedit->setWindowTitle(tr("Memory viewer"));
    addDockWidget(Qt::TopDockWidgetArea, dock_ramedit);

    QSettings settings("Gekko team", "Gekko");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());

    // menu items
    QMenu* filebrowser_menu = ui.menu_View->addMenu(tr("File browser layout"));
    filebrowser_menu->addAction(image_info->toggleViewAction());

    QMenu* debug_menu = ui.menu_View->addMenu(tr("Debugging"));
    debug_menu->addAction(disasm->toggleViewAction());
    debug_menu->addAction(gekko_regs->toggleViewAction());
    debug_menu->addAction(callstack->toggleViewAction());
    debug_menu->addAction(dock_ramedit->toggleViewAction());

    // setup connections
    connect(ui.actionLoad_Image, SIGNAL(triggered()), this, SLOT(OnMenuLoadImage()));
    connect(ui.action_Start, SIGNAL(triggered()), this, SLOT(OnStartGame()));
    connect(ui.treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnFileBrowserDoubleClicked(const QModelIndex&)));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnFileBrowserSelectionChanged()));

    // TODO: Enable this?
//    setUnifiedTitleAndToolBarOnMac(true);
}

GMainWindow::~GMainWindow()
{
    delete emu_thread;
}

void GMainWindow::BootGame(const char* filename)
{
    emu_thread = new EmuThread(filename);
    emu_thread->start();
}

void GMainWindow::OnMenuLoadImage()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Image"), QString(), QString());
    if (filename.size())
       BootGame(filename.toLatin1().data());
}

void GMainWindow::OnStartGame()
{
    if (!ui.treeView->selectionModel()->hasSelection())
    {
        BootGame(common::g_config->default_boot_file());
    }
    else
    {
        BootGame(file_browser_model->filePath(ui.treeView->selectionModel()->currentIndex()).toLatin1().data());
    }
}

void GMainWindow::OnFileBrowserDoubleClicked(const QModelIndex& index)
{
    if (!file_browser_model->isDir(index))
    {
        // Start emulation
        BootGame(file_browser_model->filePath(ui.treeView->selectionModel()->currentIndex()).toLatin1().data());
    }
    else
    {
        // Change directory
        // TODO: Sometimes, trying to access a directory with lacking read permissions will break stuff (you'll end up in an empty directory with now way to cd back to parent directory)
        QString new_path = file_browser_model->filePath(index);
        file_browser_model->setRootPath(new_path);
        ui.treeView->setRootIndex(file_browser_model->index(new_path));
    }
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

void GMainWindow::OnFileBrowserSelectionChanged()
{
// TODO: Make ReadGCMInfo cross platform...
#if 0
// TODO: Causes instability?
	unsigned long size;
	u8 banner[0x1960];
	u8 header[0x440];
	QModelIndex index = ui.treeView->selectionModel()->currentIndex();
	if (dvd::ReadGCMInfo(file_browser_model->filePath(index).toLatin1().data(), &size, (void*)banner, (void*)header) != E_OK)
		return;

	QPixmap pm_banner = BrowserAddBanner(banner);
	image_info->SetBanner(pm_banner);
#endif
}

void GMainWindow::closeEvent(QCloseEvent* event)
{
    // Save window layout
    // NOTE: For manually created objects, you'll need to assign names via setObjectName for this to work 
    QSettings settings("Gekko team", "Gekko");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());

    QWidget::closeEvent(event);
}


#ifdef main
#undef main
#endif

int __cdecl main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GMainWindow main_window;


    char program_dir[MAX_PATH];
    _getcwd(program_dir, MAX_PATH-1);
    size_t cwd_len = strlen(program_dir);
    program_dir[cwd_len] = '/';
    program_dir[cwd_len+1] = '\0';

    common::ConfigManager config_manager;
    config_manager.set_program_dir(program_dir, MAX_PATH);
    config_manager.ReloadConfig(NULL);
    core::SetConfigManager(&config_manager);

    main_window.show();
    return app.exec();
}
