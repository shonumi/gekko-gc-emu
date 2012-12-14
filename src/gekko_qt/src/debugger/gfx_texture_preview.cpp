#include "gfx_texture_preview.hxx"

TexturePreviewManager* TexturePreviewManager::instance = NULL;

TexturePreviewWidget::TexturePreviewWidget(QWidget* parent) : QDockWidget(parent)
{
    ui.setupUi(this);

    OnAvailableSourcesListChanged();
}

void TexturePreviewWidget::OnAvailableSourcesListChanged()
{
    // TODO: What to do when the current selection is unavailable?

    ui.previewSourceBox->clear();
    ui.previewSourceBox->addItems(TexturePreviewManager::GetInstance()->GetAvailableSourcesList());
}

TexturePreviewSource::TexturePreviewSource(const QString& name)
{
    connect(this, SIGNAL(TextureAvailable()), TexturePreviewManager::GetInstance(), SLOT(OnSourceAvailable()));
    connect(this, SIGNAL(TextureUnavailable()), TexturePreviewManager::GetInstance(), SLOT(OnSourceUnavailable()));
}

TexturePreviewSource::~TexturePreviewSource()
{
    emit TextureUnvailable();
}

void TexturePreviewManager::Init(QObject* parent)
{
    instance = new TexturePreviewManager(parent);
}

TexturePreviewManager::~TexturePreviewManager()
{
}

const QStringList TexturePreviewManager::GetAvailableSourcesList()
{
    // TODO: Return reference instead?
    return QStringList();
}

void TexturePreviewManager::OnSourceAvailable()
{
    emit AvailableSourcesListChanged();
}

void TexturePreviewManager::OnSourceUnavailable()
{
    emit AvailableSourcesListChanged();
}
