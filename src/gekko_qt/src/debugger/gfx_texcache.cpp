#include "gfx_texcache.hxx"
#include "gfx_texture_preview.hxx"

#include "memory.h"

GTexcacheWidget::GTexcacheWidget(QWidget* parent) : QDockWidget(parent)
{
    Ui::TexCache ui;
    ui.setupUi(this);

    GTexcacheModel* model = new GTexcacheModel(this);
    ui.texturesList->setModel(model);

    connect(this, SIGNAL(Update()), model, SLOT(Update()));
}

GTexcacheModel::GTexcacheModel(QObject* parent) : QAbstractItemModel(parent)
{
}

void GTexcacheModel::SetTextures(const std::unordered_map<common::Hash64, TextureCache::CacheEntry>& _textures)
{
    auto old_size = textures.size();

    beginResetModel();

    textures = _textures;

    while (!sources.empty())
    {
        delete sources.back();
        sources.pop_back();
    }

    int index = 0;
    for (auto it = textures.begin(); it != textures.end(); ++it)
    {
        const TextureCache::CacheEntry& entry = it->second;
        TexturePreviewSourceFromRam* source = new TexturePreviewSourceFromRam(&Mem_RAM[entry.address & RAM_MASK], entry.width, entry.height, entry.format,
                                                                              QString("Texcache entry %1 @ 0x%2").arg(++index).arg(entry.address, 8, 16, QLatin1Char('0')),
                                                                              this);
        source->SetAvailableOrChanged();
        sources.push_back(source);
    }

    endResetModel();
}

QModelIndex GTexcacheModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column, (void*)NULL);
}

QModelIndex GTexcacheModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int GTexcacheModel::rowCount(const QModelIndex& parent) const
{
    return textures.size();
}

int GTexcacheModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant GTexcacheModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= textures.size() || index.column() >= 2)
        return QVariant();

    const TextureCache::CacheEntry& entry = GetEntry(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
            if (index.column() == 0)
                return QVariant::fromValue(QString("0x%1").arg(entry.address, 8, 16, QLatin1Char('0')));
            else if (index.column() == 1)
                return QVariant::fromValue(QString("0x%1").arg(entry.hash, 8, 16, QLatin1Char('0')));

        case Qt::TextAlignmentRole:
            return Qt::AlignRight;

        default:
            return QVariant();
    }
}

QVariant GTexcacheModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
            return tr("Address");
        if (section == 1)
            return tr("Hash");
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

const TextureCache::CacheEntry& GTexcacheModel::GetEntry(int index) const
{
    auto it = textures.begin();

    for (int i = 0;i < index; ++i)
        ++it;

    return it->second;
}

#include "video_core.h"
#include "renderer_gl3/renderer_gl3.h"

void GTexcacheModel::Update()
{
    SetTextures(((RendererGL3*)video_core::g_renderer)->texture_cache_->cache_->hash_map_);
}
