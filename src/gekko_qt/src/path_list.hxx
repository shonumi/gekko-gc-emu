#ifndef _PATH_LIST_HXX_
#define _PATH_LIST_HXX_

#include <QListWidget>

class GPathList : public QListWidget
{
    Q_OBJECT

public:
    GPathList(QWidget* parent = NULL);

public slots:
    void AddPath(const QString& path);
};

#endif // _PATH_LIST_HXX_
