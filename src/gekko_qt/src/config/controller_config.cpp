#include "controller_config.hxx"
#include "controller_config_util.hxx"

using common::Config;

GControllerConfig::GControllerConfig(common::Config::ControllerPort* initial_config, QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    ((QGridLayout*)ui.mainStickTab->layout())->addWidget(new GStickConfig(Config::ANALOG_LEFT, Config::ANALOG_RIGHT, Config::ANALOG_UP, Config::ANALOG_DOWN, this), 1, 1);
    ((QGridLayout*)ui.cStickTab->layout())->addWidget(new GStickConfig(Config::C_LEFT, Config::C_RIGHT, Config::C_UP, Config::C_DOWN, this), 1, 1);
    ((QGridLayout*)ui.dPadTab->layout())->addWidget(new GStickConfig(Config::DPAD_LEFT, Config::DPAD_RIGHT, Config::DPAD_UP, Config::DPAD_DOWN, this), 1, 1);

    // TODO: Arrange these more compactly?
    QVBoxLayout* layout = (QVBoxLayout*)ui.buttonsTab->layout();
    layout->addWidget(new GButtonConfigGroup("A Button", Config::BUTTON_A, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("B Button", Config::BUTTON_B, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("X Button", Config::BUTTON_X, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("Y Button", Config::BUTTON_Y, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("Z Button", Config::BUTTON_Z, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("L Trigger", Config::TRIGGER_L, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("R Trigger", Config::TRIGGER_R, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("Start Button", Config::BUTTON_START, ui.buttonsTab));

    memcpy(config, initial_config, sizeof(config));
}

void GControllerConfig::OnKeyConfigChanged(common::Config::Control id, int key, const QString& name)
{
    if (InputSourceJoypad())
    {
        config[GetActiveController()].pads.key_code[id] = key;
    }
    else
    {
        config[GetActiveController()].keys.key_code[id] = key;
    }
}

int GControllerConfig::GetActiveController()
{
    return ui.activeControllerCB->currentIndex();
}

bool GControllerConfig::InputSourceJoypad()
{
    return ui.inputSourceCB->currentIndex() == 1;
}
