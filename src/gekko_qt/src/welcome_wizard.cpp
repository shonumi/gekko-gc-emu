#include <QFileDialog>
#include "welcome_wizard.hxx"

GWelcomeWizard::GWelcomeWizard(QWidget* parent): QWizard(parent)
{
    ui.setupUi(this);

    ui.button_browse_path->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
//    ui.button_add_path->setIcon(); // TODO: Use an icon instead of text
    ui.button_add_path->setText(tr("Add Path"));

    connect(ui.button_browse_path, SIGNAL(clicked()), this, SLOT(OnBrowsePath()));
    connect(ui.button_add_path, SIGNAL(clicked()), this, SLOT(OnAddPath()));

    connect(this, SIGNAL(PathBrowsed(const QString&)), ui.edit_path, SLOT(setText(const QString&)));
    connect(this, SIGNAL(PathAdded(const QString&)), ui.path_list, SLOT(AddPath(const QString&)));

    show();
}

void GWelcomeWizard::OnBrowsePath()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Browse for Path"));
    if (dir.size())
        ui.edit_path->setText(dir);
}

void GWelcomeWizard::OnAddPath()
{
    if (ui.edit_path->text().isEmpty())
        return;

    emit PathAdded(ui.edit_path->text());
    ui.edit_path->clear();
}
