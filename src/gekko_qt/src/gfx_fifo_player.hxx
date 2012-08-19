#ifndef _GEKKO_GFX_FIFO_PLAYER_HXX_
#define _GEKKO_GFX_FIFO_PLAYER_HXX_

#include <QDockWidget>
#include "ui_gfx_fifo_player.h"

namespace fifo_player {
    struct FPFile;
}

class GGfxFifoPlayerControl : public QDockWidget
{
    Q_OBJECT

public:
    GGfxFifoPlayerControl(QWidget* parent = NULL);
    ~GGfxFifoPlayerControl();

private slots:
    void OnStartStopRecordingClicked();
    void OnSaveRecordingClicked();

    void OnStartPlaybackClicked();

private:
    enum {
        PLAYBACK_SOURCE_LAST_RECORDING = 0,
        PLAYBACK_SOURCE_FROM_FILE = 1,
    };

    Ui::GfxFifoPlayerControl ui;

    const fifo_player::FPFile* last_recording;
};

#endif // _GEKKO_GFX_FIFO_PLAYER_HXX_
