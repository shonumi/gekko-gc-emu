#ifndef _WELCOME_WIZARD_HXX_
#define _WELCOME_WIZARD_HXX_

#include <QWizard>
#include "ui_welcome_wizard.h"

class GWelcomeWizard : public QWizard
{
    Q_OBJECT

public:
    GWelcomeWizard(QWidget* parent = NULL);

public slots:
    void OnBrowsePath();
    void OnAddPath();

signals:
    void PathBrowsed(const QString&);
    void PathAdded(const QString&);

private:
    Ui::WelcomeWizard ui;
};

#endif // _WELCOME_WIZARD_HXX_
