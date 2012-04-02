#include "path_list.hxx"

GPathList::GPathList(QWidget* parent) : QListWidget(parent)
{
}

void GPathList::AddPath(const QString& path)
{
    if (findItems(path, Qt::MatchFixedString).size())
        return;

    addItem(path);
    sortItems(Qt::AscendingOrder);
}
