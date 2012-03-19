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
#include "bootmanager.hxx"
#include "hotkeys.h"

#include "core.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
#include "version.h"


GMainWindow::GMainWindow()
{
    ui.setupUi(this);
    statusBar()->hide();

    // TODO: Make drives show up as well...
    QString sPath = QDir::currentPath();
    file_browser_model = new QFileSystemModel(this);
    file_browser_model->setFilter(QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDot);
    file_browser_model->setRootPath(sPath);

    ui.treeView->setModel(file_browser_model);
    ui.treeView->setRootIndex(file_browser_model->index(sPath));
    ui.treeView->sortByColumn(0, Qt::AscendingOrder);
    ui.treeView->hideColumn(2); // drive
    ui.treeView->hideColumn(3); // date

    render_window = new GRenderWindow;
    render_window->hide();

    // create custom widgets
    image_info = new GImageInfo(this);
    addDockWidget(Qt::RightDockWidgetArea, image_info);

    GDisAsmView* disasm = new GDisAsmView(this, render_window->GetEmuThread());
    addDockWidget(Qt::TopDockWidgetArea, disasm);

    GGekkoRegsView* gekko_regs = new GGekkoRegsView(this);
    addDockWidget(Qt::LeftDockWidgetArea, gekko_regs);

    GCallstackView* callstack = new GCallstackView(this);
    addDockWidget(Qt::BottomDockWidgetArea, callstack);

    QDockWidget* dock_ramedit = new QDockWidget(this);
    dock_ramedit->setObjectName("RamViewer");
    GRamView* ram_edit = new GRamView(dock_ramedit);
    dock_ramedit->setWidget(ram_edit);
    dock_ramedit->setWindowTitle(tr("Memory viewer"));
    addDockWidget(Qt::TopDockWidgetArea, dock_ramedit);

    // menu items
    QMenu* filebrowser_menu = ui.menu_View->addMenu(tr("File browser layout"));
    filebrowser_menu->addAction(image_info->toggleViewAction());

    QMenu* debug_menu = ui.menu_View->addMenu(tr("Debugging"));
    debug_menu->addAction(disasm->toggleViewAction());
    debug_menu->addAction(gekko_regs->toggleViewAction());
    debug_menu->addAction(callstack->toggleViewAction());
    debug_menu->addAction(dock_ramedit->toggleViewAction());

    // restore UI state
    QSettings settings("Gekko team", "Gekko");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    render_window->restoreGeometry(settings.value("geometryRenderWindow").toByteArray());

    // setup connections
    connect(ui.actionLoad_Image, SIGNAL(triggered()), this, SLOT(OnMenuLoadImage()));
    connect(ui.action_Start, SIGNAL(triggered()), this, SLOT(OnStartGame()));
    connect(ui.actionSingle_Window_Mode, SIGNAL(triggered(bool)), this, SLOT(SetupEmuWindowMode()));
    connect(ui.treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnFileBrowserDoubleClicked(const QModelIndex&)));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnFileBrowserSelectionChanged()));

    // BlockingQueuedConnection is important here, it makes sure we've finished refreshing our views before the CPU continues
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), ram_edit, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), disasm, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), gekko_regs, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), callstack, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);

    // Setup hotkeys
    Hotkeys::LoadHotkeys(settings);
    connect(Hotkeys::GetShortcut(Hotkeys::Disasm_Step, this), SIGNAL(activated()), disasm, SLOT(OnStep()));
//    connect(Hotkeys::GetShortcut(Hotkeys::Disasm_StepInto, this), SIGNAL(activated()), disasm, SLOT(OnStepInto()));
//    connect(Hotkeys::GetShortcut(Hotkeys::Disasm_Pause, this), SIGNAL(activated()), disasm, SLOT(OnPause()));
    connect(Hotkeys::GetShortcut(Hotkeys::Disasm_Cont, this), SIGNAL(activated()), disasm, SLOT(OnContinue()));
    connect(Hotkeys::GetShortcut(Hotkeys::Disasm_Breakpoint, this), SIGNAL(activated()), disasm, SLOT(OnSetBreakpoint()));

    // TODO: Enable this?
//    setUnifiedTitleAndToolBarOnMac(true);
}

GMainWindow::~GMainWindow()
{
    // will get automatically deleted otherwise
    if (render_window->parent() == NULL)
        delete render_window;
}

void GMainWindow::BootGame(const char* filename)
{
    render_window->DoneCurrent(); // make sure EmuThread can access GL context
    render_window->GetEmuThread().SetFilename(filename);
    render_window->GetEmuThread().start();
    SetupEmuWindowMode();
    render_window->show();
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
        if (!render_window->GetEmuThread().isRunning())
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

static inline u32 DeccoreRGB5A3(u16 _data)
{
    u8 r, g, b, a;

    if(_data & SIGNED_BIT16) // rgb5
    {
        r = (u8)(255.0f * (((_data >> 10) & 0x1f) / 32.0f));
        g = (u8)(255.0f * (((_data >> 5) & 0x1f) / 32.0f));
        b = (u8)(255.0f * ((_data & 0x1f) / 32.0f));
        a = 0xff;	
    }else{ // rgb4a3
        r = 17 * ((_data >> 8) & 0xf);
        g = 17 * ((_data >> 4) & 0xf);
        b = 17 * (_data & 0xf);
        a = (u8)(255.0f * (((_data >> 12) & 7) / 8.0f));
    }
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void DecodeBanner(u8* src, u8* dst, int w, int h) {
    u32 *dst32 = (u32*)dst;
    u16	*src16 = ((u16*)src);
    u32	*src32 = ((u32*)src);
    int	x, y, dx, dy, i = 0, j = 0;
    int width = (w + 3) & ~3;

    for (i=0; i < w*h/2; i++) {
        src32[i] = BSWAP32(src32[i]);
    }

    for(y = 0; y < h; y += 4) {
        for(x = 0; x < width; x += 4) {
            for(dy = 0; dy < 4; dy++) {
                for(dx = 0; dx < 4; dx++) {
                    // memory is not already swapped.. use this to grab high word first
                    j ^= 1;
                    // decode color
                    dst32[width * (y + dy) + x + dx] = DeccoreRGB5A3((*((u16*)(src16 + j))));
                    dst32[width * (y + dy) + x + dx] |= 0xff000000; // Remove this for alpha (looks sh**)
                    // only increment every other time otherwise you get address doubles
                    if(!j) src16 += 2;
                }
            }
        }
    }
}

static void BrowserAddBanner(u8 *banner, QPixmap& out_pixmap)
{   
    u8			*imageA;
    imageA = (u8*)malloc(DVD_BANNER_WIDTH*DVD_BANNER_HEIGHT*4);
    DecodeBanner(banner, imageA, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT);

    out_pixmap.convertFromImage(QImage(imageA, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT, 
        QImage::Format_ARGB32));
    delete[] imageA;
}

void GMainWindow::OnFileBrowserSelectionChanged()
{
    unsigned long size;
    u8 banner[0x1960];
    dvd::GCMHeader header;
    QModelIndex index = ui.treeView->selectionModel()->currentIndex(); // TODO: this doesn't quite work..
	if (dvd::ReadGCMInfo(file_browser_model->filePath(index).toLatin1().data(), &size, (void*)banner, &header) != E_OK)
    {
        image_info->SetBanner(QPixmap());
        image_info->SetName(QString());
        image_info->SetId(QString());
        image_info->SetDeveloper(QString());
        return;
    }

    // TODO: Banner loading is broken.. colors are messed up
    QPixmap pm_banner;
    BrowserAddBanner(&banner[0x20], pm_banner);
    image_info->SetBanner(pm_banner);

    // TODO: Support SHIFT-JIS ...
    image_info->SetName(QString::fromLatin1((char*)&banner[0x1860]));
//    image_info->SetId(QString::fromLatin1(???)); // TODO
    image_info->SetDeveloper(QString::fromLatin1((char*)&banner[0x18a0]));
}

void GMainWindow::SetupEmuWindowMode()
{
    if (!render_window->GetEmuThread().isRunning())
        return;

    bool enable = ui.actionSingle_Window_Mode->isChecked();
    if (enable && render_window->parent() == NULL) // switch to single window mode
    {
        render_window->BackupGeometry();
        ui.treeView->hide();
        ui.horizontalLayout->addWidget(render_window);
        render_window->setVisible(true);
        render_window->DoneCurrent();
    }
    else if (!enable && render_window->parent() != NULL) // switch to multiple windows mode
    {
        ui.horizontalLayout->removeWidget(render_window);
        ui.treeView->show();
        render_window->setParent(NULL);
        render_window->setVisible(true);
        render_window->DoneCurrent();
        render_window->RestoreGeometry();
    }
}

void GMainWindow::closeEvent(QCloseEvent* event)
{
    // Save window layout
    // NOTE: For manually created objects, you'll need to assign names via setObjectName for this to work 
    QSettings settings("Gekko team", "Gekko");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("geometryRenderWindow", render_window->saveGeometry());
    Hotkeys::SaveHotkeys(settings);
    // TODO: Save "single window mode" check state

    render_window->close();

    QWidget::closeEvent(event);
}


#ifdef main
#undef main
#endif

int __cdecl main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_X11InitThreads);
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
