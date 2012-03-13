#include <QDockWidget>
#include "ui_image_info.h"

class GImageInfo : public QDockWidget
{
	Q_OBJECT

public:
	GImageInfo(QWidget* parent = NULL);

	void SetBanner(const QPixmap& banner);
	void SetName(const char* name);

private:
	Ui::ImageInfo ui;

    QPixmap banner;
};
