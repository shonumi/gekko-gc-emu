#include <QDockWidget>
#include "ui_image_info.h"

class GImageInfo : public QDockWidget
{
	Q_OBJECT

public:
	GImageInfo(QWidget* parent = NULL);

	void SetBanner(const QPixmap& banner)
	{
		ui.label_banner->setPixmap(banner);
	}

	void SetName(const char* name)
	{
		ui.line_name->setText(QString::fromLatin1(name));
	}

private:
	Ui::ImageInfo ui;
};
