#ifndef _IMAGE_INFO_HXX_
#define _IMAGE_INFO_HXX_

#include <QDockWidget>
#include "ui_image_info.h"

class GImageInfo : public QDockWidget
{
    Q_OBJECT

public:
    GImageInfo(QWidget* parent = NULL);

    // keeps a reference to the parameter, so don't delete it!
    void SetBanner(const QPixmap& banner);

    void SetName(const QString& name);
    void SetId(const QString& id);
    void SetDeveloper(const QString& developer);
    void SetDescription(const QString& description);

private:
    Ui::ImageInfo ui;
};

#endif // _IMAGE_INFO_HXX_
