#include <QDockWidget>
#include <QAbstractItemModel>
#include "ui_gfx_texcache.h"

#include "renderer_gl3/texture_cache.h"

class GTexcacheWidget : public QDockWidget
{
    Q_OBJECT

public:
    GTexcacheWidget(QWidget* parent);

signals:
    void Update();
private:
};

class TexturePreviewSourceFromRam;
class GTexcacheModel : public QAbstractItemModel
{
    Q_OBJECT // TODO: Probably doesn't need to be a Q_OBJECT

public:
    GTexcacheModel(QObject* parent);

    enum
    {
        Role_CacheEntry = Qt::UserRole,
        Role_Hash = Qt::UserRole+1,
    };

    void SetTextures(const std::unordered_map<common::Hash64, TextureCache::CacheEntry>& textures);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void Update();

private:
    const TextureCache::CacheEntry& GetEntry(int index) const;

    std::unordered_map<common::Hash64, TextureCache::CacheEntry> textures;
    std::vector<TexturePreviewSourceFromRam*> sources;
};
