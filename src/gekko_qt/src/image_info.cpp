#include "image_info.hxx"

GImageInfo::GImageInfo(QWidget* parent) : QDockWidget(parent)
{
	ui.setupUi(this);
}

void GImageInfo::SetBanner(const QPixmap& banner)
{
    // NOTE: We need to make sure not to pass the elusive reference as a permanent pixmap,
    //       thus we're storing an own pixmap
    this->banner = banner;
	ui.label_banner->setPixmap(this->banner);
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