#ifndef _GEKKO_GFX_FIFO_PLAYER_HXX_
#define _GEKKO_GFX_FIFO_PLAYER_HXX_

#include <QDockWidget>
#include "ui_gfx_fifo_player.h"

class GGfxFifoPlayerControl : public QDockWidget
{
    Q_OBJECT

public:
    GGfxFifoPlayerControl(QWidget* parent = NULL);
    ~GGfxFifoPlayerControl();

private:
    Ui::GfxFifoPlayerControl ui;
};

#endif // _GEKKO_GFX_FIFO_PLAYER_HXX_
