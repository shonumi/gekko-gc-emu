#include "image_info.hxx"

GImageInfo::GImageInfo(QWidget* parent) : QDockWidget(parent)
{
    ui.setupUi(this);
}

void GImageInfo::SetBanner(const QPixmap& banner)
{
    ui.label_banner->setPixmap(banner);
}

void GImageInfo::SetName(const QString& name)
{
    ui.line_name->setText(name);
}

void GImageInfo::SetId(const QString& id)
{
    ui.line_gameid->setText(id);
}

void GImageInfo::SetDeveloper(const QString& developer)
{
    ui.line_developer->setText(developer);
}

void GImageInfo::SetDescription(const QString& description)
{
    ui.edit_description->setPlainText(description);
}
