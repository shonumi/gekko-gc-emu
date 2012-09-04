#include "controller_config.hxx"
#include "controller_config_util.hxx"

GControllerConfig::GControllerConfig(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    ((QGridLayout*)ui.mainStickTab->layout())->addWidget(new GStickConfig(MAINSTICK_LEFT, MAINSTICK_RIGHT, MAINSTICK_UP, MAINSTICK_DOWN, this), 1, 1);
    ((QGridLayout*)ui.cStickTab->layout())->addWidget(new GStickConfig(CSTICK_LEFT, CSTICK_RIGHT, CSTICK_UP, CSTICK_DOWN, this), 1, 1);
    ((QGridLayout*)ui.dPadTab->layout())->addWidget(new GStickConfig(DPAD_LEFT, DPAD_RIGHT, DPAD_UP, DPAD_DOWN, this), 1, 1);

    // TODO: Arrange these more compactly?
    QVBoxLayout* layout = (QVBoxLayout*)ui.buttonsTab->layout();
    layout->addWidget(new GButtonConfigGroup("A Button", BUTTON_A, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("B Button", BUTTON_B, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("X Button", BUTTON_X, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("Y Button", BUTTON_Y, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("Z Button", BUTTON_Z, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("L Trigger", TRIGGER_L, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("R Trigger", TRIGGER_R, ui.buttonsTab));
    layout->addWidget(new GButtonConfigGroup("Start Button", BUTTON_START, ui.buttonsTab));
}
