#include <QMainWindow>

#include "ui_main.h"

class QFileSystemModel;
class GImageInfo;

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

private slots:
    void OnStartGame();
    void OnFileBrowserDoubleClicked(const QModelIndex&);
    void OnFileBrowserClicked(const QModelIndex&);

signals:
//    UpdateUIState();

private:
    Ui::MainWindow ui;
    QFileSystemModel* file_browser_model;
	GImageInfo* image_info;
};
