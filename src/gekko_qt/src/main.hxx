#include <QMainWindow>

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

signals:
//    UpdateUIState();
};
