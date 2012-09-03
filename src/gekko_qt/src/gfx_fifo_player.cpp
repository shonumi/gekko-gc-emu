#include <QFileDialog>

#include "gfx_fifo_player.hxx"

#include "fifo_player.h"

GGfxFifoPlayerControl::GGfxFifoPlayerControl(QWidget* parent) : QDockWidget(parent), last_recording(NULL)
{
    ui.setupUi(this);

    connect(ui.startStopRecordingButton, SIGNAL(clicked()), this, SLOT(OnStartStopRecordingClicked()));
    connect(ui.saveRecordingButton, SIGNAL(clicked()), this, SLOT(OnSaveRecordingClicked()));
    connect(ui.startPlaybackButton, SIGNAL(clicked()), this, SLOT(OnStartPlaybackClicked()));
}

GGfxFifoPlayerControl::~GGfxFifoPlayerControl()
{
    if (fifo_player::IsRecording())
        fifo_player::EndRecording();
}

void GGfxFifoPlayerControl::OnStartStopRecordingClicked()
{
    if (!fifo_player::IsRecording())
    {
        ui.saveRecordingButton->setEnabled(false);
        ui.playbackGroup->setEnabled(false);
        ui.startStopRecordingButton->setText(tr("Stop"));
        // TODO: Adjust parameters
        fifo_player::StartRecording();
    }
    else
    {
        last_recording = &fifo_player::EndRecording();
        // TODO: Do other stuff

        ui.startStopRecordingButton->setText(tr("Start"));
        ui.playbackGroup->setEnabled(true); // TODO: Shouldn't do that while playing back
        ui.saveRecordingButton->setEnabled(true);
    }
}

void GGfxFifoPlayerControl::OnSaveRecordingClicked()
{
    // TODO
    QString filename = QFileDialog::getOpenFileName(this, tr("Save Fifo log"), QString(), QString());
    if (filename.size())
        fifo_player::Save(filename.toLatin1().data(), (fifo_player::FPFile&)*last_recording);
    ui.playbackGroup->setEnabled(true);
}

void GGfxFifoPlayerControl::OnStartPlaybackClicked()
{
    switch (ui.playbackSourceCombobox->currentIndex())
    {
        case PLAYBACK_SOURCE_LAST_RECORDING:
        {
            fifo_player::PlayFile((fifo_player::FPFile&)*last_recording);
            break;
        }

        case PLAYBACK_SOURCE_FROM_FILE:
        {
            QString filename = QFileDialog::getOpenFileName(this, tr("Save Fifo log"), QString(), QString());
            if (filename.size())
            {
                fifo_player::FPFile file;
                fifo_player::Load(filename.toLatin1().data(), file);
                fifo_player::PlayFile(file);
                break;
            }
            else return;
        }
    }

    ui.playbackGroup->setEnabled(false);
    // TODO: Re-enable playback group once playback is finished
}
