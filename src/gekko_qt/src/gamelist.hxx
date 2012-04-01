#ifndef _GAMELIST_HXX_
#define _GAMELIST_HXX_

#include <QAbstractItemModel>
#include <QTreeView>
#include <QTableView>
#include <QVector>
#include <QMap>
#include <QString>

#include "types.h"
#include "dvd/gcm.h"

class QFileSystemModel;
class QStandardItemModel;
class IsoInfo;
class QString;


class IsoInfo
{
public:
    QString filename;
    QString name;
    QString unique_id;
    QString developer;
    QString description;
    u8 banner[DVD_BANNER_WIDTH*DVD_BANNER_HEIGHT*4]; // TODO: Can probably be removed...
    QPixmap pm;
};
Q_DECLARE_METATYPE(IsoInfo)

class IsoList
{
public:
    const QVector<IsoInfo>& GetEntries() const;

    void AddPath(const QString& path);
    void RemovePath(const QString& path);
    void ClearPathList();

    // TODO: Those might be useful in the future
//    void AddSingleIso(const QString& filename);
//    void RemoveSingleIso(const QString& filename);

    void UpdateEntries();

private:
    QVector<IsoInfo> entries;
    QVector<QString> paths;
    bool entries_dirty;
};

class GGameBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Mode
    {
        Mode_Table,
        Mode_List,
    };
    enum
    {
        Role_IsoInfo = Qt::UserRole,
        Role_IsValid = Qt::UserRole+1,
    };

    GGameBrowserModel(QWidget* parent = NULL);

    void SetMode(Mode mode);
    void Browse(QString path);
    void SetNumColumns(int columns);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

private:
    Mode mode;
    IsoList isolist;
    int columns;
};

// TODO: This basically is the common interface for game browsers.
// However, Qt wouldn't let me make the other classes inherit GGameBrowser, so it's not really used at the moment...
class GGameBrowser
{
public:
    enum Style {
        Style_Table,
//        Style_List,
        Style_FileBrowser,
        Style_None,
    };

    virtual ~GGameBrowser() {}

//public slots:
//    virtual bool HasSelection() = 0;

    // make sure HasSelection returns true before calling this
//    virtual IsoInfo SelectedIso() = 0;

//    void Browse(const QString& path) = 0;

//signals:
//    void IsoSelected(const IsoInfo& info);
//    void EmuStartRequested(const IsoInfo& info);
};


class GGameTable : public QTableView
{
    Q_OBJECT

public:
    GGameTable(QWidget* parent = NULL);

    void resizeEvent(QResizeEvent*);

public slots:
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void OnSelectionChanged(const QItemSelection& current);
    void OnDoubleClicked(const QModelIndex& index);

    void Browse(const QString& path);
    bool HasSelection();
    IsoInfo SelectedIso();

signals:
    void IsoSelected(const IsoInfo& info);
    void EmuStartRequested();

private:
    GGameBrowserModel* model;
};

class GGameFileBrowser : public QTreeView
{
    Q_OBJECT

public:
    GGameFileBrowser(QWidget* parent = NULL);

public slots:
    void OnSelectionChanged(const QItemSelection& current);
    void OnDoubleClicked(const QModelIndex& index);
    bool HasSelection();
    IsoInfo SelectedIso();

signals:
    void IsoSelected(const IsoInfo& info);
    void EmuStartRequested();

private:
    QFileSystemModel* model;
    IsoInfo selected_iso;
};

#endif // GAMELIST_HXX_
