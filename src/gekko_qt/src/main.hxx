#ifndef _GEKKO_QT_MAIN_HXX_
#define _GEKKO_QT_MAIN_HXX_

#include <QMainWindow>

#include "ui_main.h"
#include "gamelist.hxx"

class GImageInfo;
class GRenderWindow;

class GMainWindow : public QMainWindow
{
    Q_OBJECT

    // TODO: Make use of this!
    enum {
        UI_IDLE,
        UI_EMU_BOOTING,
        UI_EMU_RUNNING,
        UI_EMU_STOPPING,
    };

public:
    GMainWindow();
    ~GMainWindow();

private:
    void BootGame(const char* filename);
    void SetGameBrowserStyle(GGameBrowser::Style style);

    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

private slots:
    void OnStartGame();
    void OnMenuLoadImage();
    void OnMenuBrowseForImages();
    void OnIsoSelected(const IsoInfo&);
    void OnOpenHotkeysDialog();
    void SetupEmuWindowMode();
    void OnChangeGameBrowserStyle(QAction* source);

private:
    Ui::MainWindow ui;
    GImageInfo* image_info;

    QWidget* game_browser;
    GRenderWindow* render_window;

    GGameBrowser::Style gbs_style;
};

#endif // _GEKKO_QT_MAIN_HXX_
