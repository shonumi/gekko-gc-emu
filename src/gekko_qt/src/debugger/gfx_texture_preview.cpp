#include "gfx_texture_preview.hxx"

TexturePreviewManager* TexturePreviewManager::instance = NULL;

TexturePreviewWidget::TexturePreviewWidget(QWidget* parent) : QDockWidget(parent)
{
    ui.setupUi(this);

    connect(ui.previewSourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSelectedPreviewSourceChanged(int)));

    connect(TexturePreviewManager::GetInstance(), SIGNAL(AvailableSourcesListChanged()), this, SLOT(OnAvailableSourcesListChanged()));

    OnAvailableSourcesListChanged();
}

void TexturePreviewWidget::OnAvailableSourcesListChanged()
{
    // TODO: What to do when the current selection is unavailable?

    ui.previewSourceBox->clear();
    ui.previewSourceBox->addItem(tr("None"));
    ui.previewSourceBox->addItems(TexturePreviewManager::GetInstance()->GetSourcesNames());
    ui.previewSourceBox->setCurrentIndex(0);
}

// TODO: Ugly.
unsigned char buf[1024*1024*4];

void TexturePreviewWidget::OnSelectedPreviewSourceChanged(int index)
{
    if (index < 1)
        return;

    // TODO: Allow different displays... RGB32, ARGB32, A8 as R8
    TexturePreviewSource* source = TexturePreviewManager::GetInstance()->GetSource(index-1);
    gp::TextureDecoder_Decode(source->GetFormat(), source->GetWidth(), source->GetHeight(), (const u8*)source->GetData(), (u8*)buf);
    ui.textureLabel->setPixmap(QPixmap::fromImage(QImage((uchar*)buf, source->GetWidth(), source->GetHeight(), source->GetWidth()*4, QImage::Format_RGB32)));

    ui.textureLabel->setScaledContents(true); // TODO: Probably don't want this scaled to the full widget, make it a multiple of the original dimensions
}

TexturePreviewSource::TexturePreviewSource(const QString& name, QObject* parent) : QObject(parent), name(name)
{
    connect(this, SIGNAL(TextureAvailable()), TexturePreviewManager::GetInstance(), SLOT(OnSourceAvailable()));
    connect(this, SIGNAL(TextureUnavailable()), TexturePreviewManager::GetInstance(), SLOT(OnSourceUnavailable()));
}

TexturePreviewSource::~TexturePreviewSource()
{
    emit TextureUnavailable();
}

TexturePreviewSourceFromRam::TexturePreviewSourceFromRam(void* data, unsigned int width, unsigned int height, gp::TextureFormat format, const QString& name, QObject* parent)
                            : TexturePreviewSource(name, parent), data(data), width(width), height(height), format(format)
{
}

void TexturePreviewSourceFromRam::SetAvailableOrChanged()
{
    emit TextureAvailable();
}

void TexturePreviewSourceFromRam::SetUnavailable()
{
    emit TextureUnavailable();
}

void TexturePreviewManager::Init(QObject* parent)
{
    instance = new TexturePreviewManager(parent);
}

TexturePreviewManager::TexturePreviewManager(QObject* parent) : QObject(parent)
{
    connect(this, SIGNAL(AvailableSourcesListChanged()), this, SLOT(RebuildSourceNamesList()));
}

TexturePreviewManager::~TexturePreviewManager()
{
}

const QStringList& TexturePreviewManager::GetSourcesNames() const
{
    return sourcesNames;
}

/*const */TexturePreviewSource* TexturePreviewManager::GetSource(unsigned int index) const
{
    return sources[index];
}


void TexturePreviewManager::OnSourceAvailable()
{
    TexturePreviewSource* source = (TexturePreviewSource*)sender();

    if (qFind(sources.begin(), sources.end(), source) != sources.end())
        return;

    sources.append(source);
    qSort(sources.begin(), sources.end()); // TODO: Sorting by ptr value here, not by string?

    emit AvailableSourcesListChanged();
}

void TexturePreviewManager::OnSourceUnavailable()
{
    TexturePreviewSource* source = (TexturePreviewSource*)sender();
    QList<TexturePreviewSource*>::iterator it = qFind(sources.begin(), sources.end(), source);

    if (it == sources.end())
        return;

    sources.erase(it);

    emit AvailableSourcesListChanged();
}

void TexturePreviewManager::RebuildSourceNamesList()
{
    sourcesNames.clear();

    QListIterator<TexturePreviewSource*> it(sources);
    while (it.hasNext())
        sourcesNames.append(it.next()->GetName());
}
