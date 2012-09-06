#include <QtGui>
#include "qhexedit.h"
#include "main.hxx"

#include "common.h"
#include "config.h"

#include "callstack.hxx"
#include "disasm.hxx"
#include "gamelist.hxx"
#include "gekko_regs.hxx"
#include "image_info.hxx"
#include "ramview.hxx"
#include "bootmanager.hxx"
#include "hotkeys.hxx"
#include "welcome_wizard.hxx"

#include "config/controller_config.hxx"

#include "core.h"
#include "dvd/loader.h"
#include "dvd/gcm.h"
#include "version.h"

common::Config::ControllerPort controller_ports[4];

GMainWindow::GMainWindow() : gbs_style(GGameBrowser::Style_None), game_browser(NULL)
{
    ui.setupUi(this);
    statusBar()->hide();

    render_window = new GRenderWindow;
    render_window->hide();

    // create custom widgets
    image_info = new GImageInfo(this);
    addDockWidget(Qt::RightDockWidgetArea, image_info);

    GDisAsmView* disasm = new GDisAsmView(this, render_window->GetEmuThread());
    addDockWidget(Qt::TopDockWidgetArea, disasm);
    disasm->hide();

    GGekkoRegsView* gekko_regs = new GGekkoRegsView(this);
    addDockWidget(Qt::LeftDockWidgetArea, gekko_regs);
    gekko_regs->hide();

    GCallstackView* callstack = new GCallstackView(this);
    addDockWidget(Qt::BottomDockWidgetArea, callstack);
    callstack->hide();

    QDockWidget* dock_ramedit = new QDockWidget(this);
    dock_ramedit->setObjectName("RamViewer");
    GRamView* ram_edit = new GRamView(dock_ramedit);
    dock_ramedit->setWidget(ram_edit);
    dock_ramedit->setWindowTitle(tr("Memory viewer"));
    addDockWidget(Qt::TopDockWidgetArea, dock_ramedit);
    dock_ramedit->hide();

    // menu items
    QMenu* filebrowser_menu = ui.menu_View->addMenu(tr("File browser layout"));
    filebrowser_menu->addAction(image_info->toggleViewAction());

    QActionGroup* action_group_gbs = new QActionGroup(this);
    action_group_gbs->addAction(ui.action_gbs_table);
    action_group_gbs->addAction(ui.action_gbs_file_browser);

    QMenu* debug_menu = ui.menu_View->addMenu(tr("Debugging"));
    debug_menu->addAction(disasm->toggleViewAction());
    debug_menu->addAction(gekko_regs->toggleViewAction());
    debug_menu->addAction(callstack->toggleViewAction());
    debug_menu->addAction(dock_ramedit->toggleViewAction());

    // Set default UI state
    // geometry: 55% of the window contents are in the upper screen half, 45% in the lower half
    QDesktopWidget* desktop = ((QApplication*)QApplication::instance())->desktop();
    QRect screenRect = desktop->screenGeometry(this);
    int x, y, w, h;
    w = screenRect.width() * 2 / 3;
    h = screenRect.height() / 2;
    x = (screenRect.x() + screenRect.width()) / 2 - w / 2;
    y = (screenRect.y() + screenRect.height()) / 2 - h * 55 / 100;
    setGeometry(x, y, w, h);


    // Restore UI state
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Gekko team", "Gekko");
    SetGameBrowserStyle((GGameBrowser::Style)settings.value("gameBrowserStyle", GGameBrowser::Style_Table).toInt());
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    render_window->restoreGeometry(settings.value("geometryRenderWindow").toByteArray());
    bool first_start = settings.value("firstStart", true).toBool();

    ui.actionSingle_Window_Mode->setChecked(settings.value("singleWindowMode", false).toBool());
    SetupEmuWindowMode();

	// Load controller settings, TODO: Move these settings to the main XML configuration and support joystick configuration
    memset(controller_ports, 0, sizeof(controller_ports));
    controller_ports[0].keys.enable = true;
    controller_ports[0].keys.key_code[common::Config::BUTTON_A] = Qt::Key_X;
    controller_ports[0].keys.key_code[common::Config::BUTTON_B] = Qt::Key_Y; // QWERTZ ftw
    controller_ports[0].keys.key_code[common::Config::BUTTON_X] = Qt::Key_A;
    controller_ports[0].keys.key_code[common::Config::BUTTON_Y] = Qt::Key_S;
    controller_ports[0].keys.key_code[common::Config::TRIGGER_L] = Qt::Key_Q;
    controller_ports[0].keys.key_code[common::Config::TRIGGER_R] = Qt::Key_W;
    controller_ports[0].keys.key_code[common::Config::BUTTON_START] = Qt::Key_Return;
    controller_ports[0].keys.key_code[common::Config::ANALOG_LEFT] = Qt::Key_Left;
    controller_ports[0].keys.key_code[common::Config::ANALOG_RIGHT] = Qt::Key_Right;
    controller_ports[0].keys.key_code[common::Config::ANALOG_UP] = Qt::Key_Up;
    controller_ports[0].keys.key_code[common::Config::ANALOG_DOWN] = Qt::Key_Down;

    int num_controllers = settings.beginReadArray("controllers");
    for (int i = 0; i < num_controllers; ++i) {
        settings.setArrayIndex(i);
        for (int control = 0; control < common::Config::NUM_CONTROLS; ++control) {
            if (settings.contains(QString("control%1").arg(control))) {
                controller_ports[i].keys.key_code[control] = settings.value(QString("control%1").arg(control)).toInt();
            }
        }
    }
    settings.endArray();

    // Setup connections
    connect(ui.action_load_image, SIGNAL(triggered()), this, SLOT(OnMenuLoadImage()));
    connect(ui.action_browse_images, SIGNAL(triggered()), this, SLOT(OnMenuBrowseForImages()));
    connect(ui.action_Start, SIGNAL(triggered()), this, SLOT(OnStartGame()));
    connect(ui.actionSingle_Window_Mode, SIGNAL(triggered(bool)), this, SLOT(SetupEmuWindowMode()));
    connect(ui.actionHotkeys, SIGNAL(triggered()), this, SLOT(OnOpenHotkeysDialog()));
    connect(action_group_gbs, SIGNAL(triggered(QAction*)), this, SLOT(OnChangeGameBrowserStyle(QAction*)));

    // BlockingQueuedConnection is important here, it makes sure we've finished refreshing our views before the CPU continues
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), ram_edit, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), disasm, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), gekko_regs, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);
    connect(&render_window->GetEmuThread(), SIGNAL(CPUStepped()), callstack, SLOT(OnCPUStepped()), Qt::BlockingQueuedConnection);

    // Setup hotkeys
    RegisterHotkey("Main Window", "Load Image", QKeySequence::Open);
    RegisterHotkey("Main Window", "Start Emulation");
    LoadHotkeys(settings);

    connect(GetHotkey("Main Window", "Load Image", this), SIGNAL(activated()), this, SLOT(OnMenuLoadImage()));
    connect(GetHotkey("Main Window", "Start Emulation", this), SIGNAL(activated()), this, SLOT(OnStartGame()));

    // TODO: Enable this?
//    setUnifiedTitleAndToolBarOnMac(true);

    show();
    if (first_start)
    {
        // TODO: This should actually modify Gekko config..
        GWelcomeWizard* wizard = new GWelcomeWizard(this);
    }
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

    // TODO: Haxx
    for (unsigned int i = 0; i < 4; ++i)
        memcpy(&common::g_config->controller_ports(i), &controller_ports[i], sizeof(common::Config::ControllerPort));

    SetupEmuWindowMode();
    render_window->show();
}

void GMainWindow::OnMenuLoadImage()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Image"), QString(), QString());
    if (filename.size())
       BootGame(filename.toLatin1().data());
}

void GMainWindow::OnMenuBrowseForImages()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Browse for Images"));
    if (dir.size())
    {
        // Qt wouldn't let me define some common interface for this, maybe Q_INTERFACE would help?
        QMetaObject::invokeMethod(game_browser, "Browse", Qt::DirectConnection, Q_ARG(const QString&, dir));
    }
}

void GMainWindow::OnStartGame()
{
    // Qt wouldn't let me define some common interface for this, maybe Q_INTERFACE would help?
    bool has_selection;
    QMetaObject::invokeMethod(game_browser, "HasSelection", Qt::DirectConnection, Q_RETURN_ARG(bool, has_selection));
    if (!has_selection)
    {
        BootGame(common::g_config->default_boot_file());
    }
    else
    {
        IsoInfo info;
        QMetaObject::invokeMethod(game_browser, "SelectedIso", Qt::DirectConnection, Q_RETURN_ARG(IsoInfo, info));
        BootGame(info.filename.toLatin1().data());
    }
}

void GMainWindow::OnOpenHotkeysDialog()
{
    GHotkeysDialog dialog(this);
    dialog.exec();
}

void GMainWindow::OnIsoSelected(const IsoInfo& info)
{
    image_info->SetBanner(info.pm);
    image_info->SetName(info.name);
    image_info->SetId(info.unique_id);
    image_info->SetDeveloper(info.developer);
    image_info->SetDescription(info.description);
}


void GMainWindow::SetupEmuWindowMode()
{
    if (!render_window->GetEmuThread().isRunning())
        return;

    bool enable = ui.actionSingle_Window_Mode->isChecked();
    if (enable && render_window->parent() == NULL) // switch to single window mode
    {
        render_window->BackupGeometry();
        game_browser->setVisible(false);
        ui.horizontalLayout->addWidget(render_window);
        render_window->setVisible(true);
        render_window->DoneCurrent();
    }
    else if (!enable && render_window->parent() != NULL) // switch to multiple windows mode
    {
        ui.horizontalLayout->removeWidget(render_window);
        game_browser->setVisible(true);
        render_window->setParent(NULL);
        render_window->setVisible(true);
        render_window->DoneCurrent();
        render_window->RestoreGeometry();
    }
}

void GMainWindow::OnChangeGameBrowserStyle(QAction* source)
{
    if (source == ui.action_gbs_file_browser)
        SetGameBrowserStyle(GGameBrowser::Style_FileBrowser);
    else if (source == ui.action_gbs_table)
        SetGameBrowserStyle(GGameBrowser::Style_Table);
}

void GMainWindow::SetGameBrowserStyle(GGameBrowser::Style style)
{
    if (style == gbs_style)
        return;

    gbs_style = style;

    if (game_browser)
    {
        ui.horizontalLayout->removeWidget(game_browser);
        delete game_browser;
        game_browser = NULL;
    }
    switch (style)
    {
        case GGameBrowser::Style_FileBrowser:
        {
            ui.action_gbs_file_browser->setChecked(true);
            game_browser = new GGameFileBrowser;
            break;
        }

        case GGameBrowser::Style_Table:
        {
            ui.action_gbs_table->setChecked(true);
            game_browser = new GGameTable;
            break;
        }
    }
    ui.horizontalLayout->addWidget(game_browser);
    connect(game_browser, SIGNAL(IsoSelected(const IsoInfo&)), this, SLOT(OnIsoSelected(const IsoInfo&)));
    connect(game_browser, SIGNAL(EmuStartRequested()), this, SLOT(OnStartGame()));
}

void GMainWindow::OnConfigure()
{
    QDialog* dialog = new QDialog(this);
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    GControllerConfig* config = new GControllerConfig(controller_ports, dialog);
    layout->addWidget(config);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));
    connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));

    layout->setSizeConstraint(QLayout::SetFixedSize);
    dialog->setLayout(layout);
    dialog->setModal(true);
    dialog->show();

    // TODO: Save and apply config when dialog closes
}

void GMainWindow::closeEvent(QCloseEvent* event)
{
    // Save window layout
    // NOTE: For manually created objects, you'll need to assign names via setObjectName for this to work 
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Gekko team", "Gekko");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("geometryRenderWindow", render_window->saveGeometry());
    settings.setValue("gameBrowserStyle", gbs_style);
    settings.setValue("singleWindowMode", ui.actionSingle_Window_Mode->isChecked());
    settings.setValue("firstStart", false);
    SaveHotkeys(settings);

    // TODO: Move this to XML config
    settings.beginWriteArray("controllers");
    for (int i = 0; i < 4; ++i) {
        settings.setArrayIndex(i);
        for (int control = 0; control < common::Config::NUM_CONTROLS; ++control) {
            settings.setValue(QString("control%1").arg(control), controller_ports[i].keys.key_code[control]);
        }
    }
    settings.endArray();

    // TODO: Should save Gekko config here

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
