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

void GControllerConfig::OnKeyConfigChanged(ControllerButtonId id, int key, const QString& name)
{
    // TODO: Clean up input_common interface mess and make this prettier
    switch (id)
    {
        case BUTTON_A:
            if (InputSourceJoypad())
            {
                // TODO
            }
            else
            {
                config[GetActiveController()].keys.key_code[common::Config::BUTTON_A] = key;
            }
            break;

        case BUTTON_B:
            break;

        case BUTTON_X:
            break;

        case BUTTON_Y:
            break;

        case BUTTON_Z:
            break;

        case TRIGGER_L:
            break;

        case TRIGGER_R:
            break;

        case BUTTON_START:
            break;

        case MAINSTICK_LEFT:
            break;

        case MAINSTICK_RIGHT:
            break;

        case MAINSTICK_UP:
            break;

        case MAINSTICK_DOWN:
            break;

        case CSTICK_LEFT:
            break;

        case CSTICK_RIGHT:
            break;

        case CSTICK_UP:
            break;

        case CSTICK_DOWN:
            break;

        case DPAD_LEFT:
            break;

        case DPAD_RIGHT:
            break;

        case DPAD_UP:
            break;

        case DPAD_DOWN:
            break;
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
