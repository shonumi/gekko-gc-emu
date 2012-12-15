#include <QDir>
#include <QFileSystemModel>
#include <QHeaderView>
#include "gamelist.hxx"

#include "dvd/loader.h"

static inline u32 DeccoreRGB5A3(u16 _data)
{
    u8 r, g, b, a;

    if(_data & SIGNED_BIT16) // rgb5
    {
        r = (u8)(255.0f * (((_data >> 10) & 0x1f) / 32.0f));
        g = (u8)(255.0f * (((_data >> 5) & 0x1f) / 32.0f));
        b = (u8)(255.0f * ((_data & 0x1f) / 32.0f));
        a = 0xff;
    }else{ // rgb4a3
        r = 17 * ((_data >> 8) & 0xf);
        g = 17 * ((_data >> 4) & 0xf);
        b = 17 * (_data & 0xf);
        a = (u8)(255.0f * (((_data >> 12) & 7) / 8.0f));
    }
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void DecodeBanner(u8* src, u8* dst, int w, int h) {
    u32 *dst32 = (u32*)dst;
    u16 *src16 = ((u16*)src);
    u32 *src32 = ((u32*)src);
    int x, y, dx, dy, i = 0, j = 0;
    int width = (w + 3) & ~3;

    for (i=0; i < w*h/2; i++) {
        src32[i] = BSWAP32(src32[i]);
    }

    for(y = 0; y < h; y += 4) {
        for(x = 0; x < width; x += 4) {
            for(dy = 0; dy < 4; dy++) {
                for(dx = 0; dx < 4; dx++) {
                    // memory is not already swapped.. use this to grab high word first
                    j ^= 1;
                    // decode color
                    dst32[width * (y + dy) + x + dx] = DeccoreRGB5A3((*((u16*)(src16 + j))));
                    dst32[width * (y + dy) + x + dx] |= 0xff000000; // Remove this for alpha (looks sh**)
                    // only increment every other time otherwise you get address doubles
                    if(!j) src16 += 2;
                }
            }
        }
    }
}

void IsoList::AddPath(const QString& path)
{
    QVector<QString>::iterator it = qFind(paths.begin(), paths.end(), path);
    if (it != paths.end())
        return;

    paths.push_back(path);
    entries_dirty = true;
}

void IsoList::RemovePath(const QString& path)
{
    QVector<QString>::iterator it = qFind(paths.begin(), paths.end(), path);
    if (it == paths.end())
        return;

    paths.erase(it);
    entries_dirty = true;
}

void IsoList::ClearPathList()
{
    paths.clear();
    entries_dirty = true;
}

void IsoList::UpdateEntries()
{
    if (!entries_dirty)
        return;

    entries.clear();
    for (QVector<QString>::iterator it = paths.begin(); it != paths.end(); ++it)
    {
        QDir dir(*it);
        QStringList files = dir.entryList(QStringList("*"), QDir::Files | QDir::Readable); // TODO: change filter..
        for (QList<QString>::iterator file = files.begin(); file != files.end(); ++file)
        {
            // TODO: Support threaded info reading
            unsigned long size;
            u8 banner[0x1960];
            dvd::GCMHeader header;
            if (dvd::ReadGCMInfo(dir.filePath(*file).toLatin1().data(), &size, (void*)banner, &header) != E_OK)
                continue;

            // TODO: Should make sure we don't have that one already.. index by filename etc
            // TODO: not compatible with SHIFT-JIS metadata..
            entries.resize(entries.size()+1);
            entries[entries.size()-1].filename = dir.filePath(*file);
            entries[entries.size()-1].name = QString::fromLatin1((char*)&banner[0x1860], qstrnlen((char*)&banner[0x1860], 0x40));
            entries[entries.size()-1].unique_id = QString::fromLatin1((char*)&header, 0x7);
            entries[entries.size()-1].developer = QString::fromLatin1((char*)&banner[0x18a0], qstrnlen((char*)&banner[0x18a0], 0x40));
            entries[entries.size()-1].description = QString::fromLatin1((char*)&banner[0x18e0], qstrnlen((char*)&banner[0x18e0], 0x80));
            DecodeBanner(&banner[0x20], entries[entries.size()-1].banner, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT);
            entries[entries.size()-1].pm = QPixmap::fromImage(QImage(entries[entries.size()-1].banner, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT,
                                                                     QImage::Format_ARGB32)).scaled(QSize(216,72), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }
    entries_dirty = false;
}

const QVector<IsoInfo>& IsoList::GetEntries() const
{
    return entries;
}

GGameBrowserModel::GGameBrowserModel(QWidget* parent) : QAbstractItemModel(parent), mode(Mode_List)
{
    SetNumColumns(1);
}

void GGameBrowserModel::SetMode(Mode mode)
{
    this->mode = mode;
}

void GGameBrowserModel::Browse(QString path)
{
    QDir dir(path);
    QStringList list = dir.entryList(QStringList("*"));

    isolist.AddPath(path);
    isolist.UpdateEntries();

    reset();
    emit dataChanged(index(0, 0), index(0, isolist.GetEntries().size()-1));
}

void GGameBrowserModel::SetNumColumns(int columns)
{
    this->columns = columns;
    reset();
    emit dataChanged(index(0, 0), index(0, isolist.GetEntries().size()-1));
}

QModelIndex GGameBrowserModel::index(int row, int column, const QModelIndex& parent) const
{
	return createIndex(row, column, (void*)NULL);
}

QModelIndex GGameBrowserModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

int GGameBrowserModel::rowCount(const QModelIndex& parent) const
{
    // TODO: Shouldn't this return a different value for game list?
    return (isolist.GetEntries().size() - 1) / columns + 1;
}

int GGameBrowserModel::columnCount(const QModelIndex& parent) const
{
    return qMin(columns, isolist.GetEntries().size());
}

QVariant GGameBrowserModel::data(const QModelIndex& index, int role) const
{
    if (index.row() * columns + index.column() >= isolist.GetEntries().size())
        return (role == Role_IsValid) ? QVariant(false) : QVariant();

    const IsoInfo& entry = isolist.GetEntries().at(index.row() * columns + index.column());
    switch (role)
    {
        case Role_IsoInfo:
            return QVariant::fromValue(entry);

        case Role_IsValid:
            return QVariant(true);

        case Qt::DecorationRole:
            return QVariant::fromValue(entry.pm);

        default:
            return QVariant();
    }
}

Qt::ItemFlags GGameBrowserModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

GGameTable::GGameTable(QWidget* parent) : QTableView(parent)
{
    model = new GGameBrowserModel(this);
    setModel(model);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setGridStyle(Qt::NoPen);

    model->SetNumColumns(qMax(1,maximumViewportSize().width() / sizeHintForColumn(0)));

    // TODO: connect pressing Enter key
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnSelectionChanged(const QModelIndex&, const QModelIndex&)));
    connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnDoubleClicked(const QModelIndex&)));
}

void GGameTable::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    resizeColumnsToContents();
    resizeRowsToContents();
    QTableView::dataChanged(topLeft, bottomRight);
}

void GGameTable::Browse(const QString& path)
{
    model->Browse(path);
}

void GGameTable::OnSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
        return;

    const IsoInfo& info = model->data(current, GGameBrowserModel::Role_IsoInfo).value<IsoInfo>();
    emit IsoSelected(info);
}

void GGameTable::OnDoubleClicked(const QModelIndex& index)
{
    if (!HasSelection())
        return;

    emit EmuStartRequested();
}

void GGameTable::resizeEvent(QResizeEvent* event)
{
    model->SetNumColumns(qMax(1,maximumViewportSize().width() / sizeHintForColumn(0)));
    QAbstractItemView::resizeEvent(event);
}

bool GGameTable::HasSelection()
{
    if (!selectionModel()->currentIndex().isValid())
        return false;

    return model->data(selectionModel()->currentIndex(), GGameBrowserModel::Role_IsValid).toBool();
}

IsoInfo GGameTable::SelectedIso()
{
    if (!selectionModel()->currentIndex().isValid())
        return IsoInfo();

    return model->data(selectionModel()->currentIndex(), GGameBrowserModel::Role_IsoInfo).value<IsoInfo>();
}


GGameFileBrowser::GGameFileBrowser(QWidget* parent): QTreeView(parent)
{
    // TODO: Make drives show up as well...
    QString sPath = QDir::currentPath();
    model = new QFileSystemModel(this);
    model->setFilter(QDir::Dirs | QDir::Files | QDir::Drives | QDir::NoDot/* | QDir::Readable*/);
    model->setRootPath(sPath);

    setModel(model);
    setRootIndex(model->index(sPath));
    setRootIsDecorated(false);
    sortByColumn(0, Qt::AscendingOrder);
    hideColumn(2); // drive
    hideColumn(3); // date

    connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnDoubleClicked(const QModelIndex&)));
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnSelectionChanged(const QModelIndex&, const QModelIndex&)));
}

void GGameFileBrowser::OnSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
    {
        selected_iso = IsoInfo();
        return;
    }

    QString filename = model->filePath(current);

    unsigned long size;
    u8 banner[0x1960];
    dvd::GCMHeader header;
    if (dvd::ReadGCMInfo(filename.toLatin1().data(), &size, (void*)banner, &header) != E_OK)
    {
        selected_iso = IsoInfo();
        return;
    }

    // TODO: not compatible with SHIFT-JIS metadata..
    selected_iso.filename = filename;
    selected_iso.name = QString::fromLatin1((char*)&banner[0x1860], qstrnlen((char*)&banner[0x1860], 0x40));
    selected_iso.unique_id = QString::fromLatin1((char*)&header, 0x7);
    selected_iso.developer = QString::fromLatin1((char*)&banner[0x18a0], qstrnlen((char*)&banner[0x18a0], 0x40));
    selected_iso.description = QString::fromLatin1((char*)&banner[0x18e0], qstrnlen((char*)&banner[0x18e0], 0x80));
    DecodeBanner(&banner[0x20], selected_iso.banner, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT);
    selected_iso.pm = QPixmap::fromImage(QImage(selected_iso.banner, DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT,
                                                QImage::Format_ARGB32)).scaled(QSize(216,72), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    emit IsoSelected(selected_iso);
}

void GGameFileBrowser::OnDoubleClicked(const QModelIndex& index)
{
    if (!model->isDir(index))
    {
        // TODO: Only if (!render_window->GetEmuThread().isRunning())
        if (HasSelection())
            emit EmuStartRequested();
    }
    else
    {
        // Change directory
        // TODO: Sometimes, trying to access a directory with lacking read permissions will break stuff (you'll end up in an empty directory with now way to cd back to parent directory)
        QString new_path = model->filePath(index);
        model->setRootPath(new_path);
        setRootIndex(model->index(new_path));
    }
}

bool GGameFileBrowser::HasSelection()
{
    if (!selectionModel()->currentIndex().isValid())
        return false;

    QString filename = model->filePath(selectionModel()->currentIndex());
    return E_OK == dvd::ReadGCMInfo(filename.toLatin1().data(), NULL, NULL, NULL);
}

IsoInfo GGameFileBrowser::SelectedIso()
{
    return selected_iso;
}
