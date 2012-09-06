#ifndef _CONTROLLER_CONFIG_HXX_
#define _CONTROLLER_CONFIG_HXX_

#include "ui_controller_config.h"

#include "config.h"

class GControllerConfig : public QWidget
{
    Q_OBJECT

public:
    GControllerConfig(QWidget* parent = NULL);

    const common::Config::ControllerPort& GetControllerConfig(int index) const { return config[index]; }

public slots:
    void OnKeyConfigChanged(common::Config::Control id, int key, const QString& name);

private:
    int GetActiveController();
    bool InputSourceJoypad();

    Ui::ControllerConfig ui;
    common::Config::ControllerPort config[4];
};

#endif  // _CONTROLLER_CONFIG_HXX_
