#ifndef _CONTROLLER_CONFIG_HXX_
#define _CONTROLLER_CONFIG_HXX_

class GStickConfig;

#include "ui_controller_config.h"

class GControllerConfig : public QWidget
{
    Q_OBJECT

public:
    GControllerConfig(QWidget* parent = NULL);

private:
    Ui::ControllerConfig ui;
};

#endif  // _CONTROLLER_CONFIG_HXX_
