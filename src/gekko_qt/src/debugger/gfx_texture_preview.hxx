#include <QDockWidget>
#include "ui_gfx_texture_preview.h"

#include "../../video_core/src/texture_decoder.h"

class TexturePreviewWidget : public QDockWidget
{
    Q_OBJECT

public:
    TexturePreviewWidget(QWidget* parent = NULL);

private slots:
    void OnAvailableSourcesListChanged();

    void OnSelectedPreviewSourceChanged(int index);

private:
    Ui::TexturePreview ui;
};

class TexturePreviewSource : public QObject
{
    Q_OBJECT

public:
    /**
     * @param name Name to be shown in texture preview source selectors
     * @todo Might need some feature to tell apart different widgets that use the same class
     */
    TexturePreviewSource(const QString& name, QObject* parent);

    virtual ~TexturePreviewSource();

    virtual const char* GetData() = 0;
    virtual gp::TextureFormat GetFormat() = 0;
    virtual unsigned int GetWidth() = 0;
    virtual unsigned int GetHeight() = 0;

    const QString& GetName() const { return name; }

    bool operator < (const TexturePreviewSource& other) { return name < other.GetName(); }

signals:
    /**
     * Signalizes that a new texture can be retrieved via GetData()
     * @note This should also be emitted when the texture changed (handlers should react correspondingly)
     * @note In particular, this makes this TexturePreviewSource show up in texture preview source selectors
     */
    void TextureAvailable();

    /**
     * Signalizes that the currently provided texture cannot be requested anymore
     * @note This should also be emitted when the texture changed (handlers should react correspondingly)
     * @note In particular, this makes this TexturePreviewSource disappear from texture preview source selectors
     */
    void TextureUnavailable();

private:
    const QString name;
};

class TexturePreviewSourceFromRam : public TexturePreviewSource
{
public:
    TexturePreviewSourceFromRam(void* data, unsigned int width, unsigned int height, gp::TextureFormat format, const QString& name, QObject* parent);

    const char* GetData() { return (const char*)data; }
    gp::TextureFormat GetFormat() { return format; }
    unsigned int GetWidth() { return width; }
    unsigned int GetHeight() { return height; }

private:
    void* data;
    unsigned int width;
    unsigned int height;
    gp::TextureFormat format;
};

class TexturePreviewManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Initializes the singleton from the parent widget. This should be called at the very top of the main window constructor.
     */
    static void Init(QObject* parent);

    /**
     * Returns the singleton instance.
     */
    static TexturePreviewManager*& GetInstance() { return instance; }

    /**
     * Returns a list of human-readable texture preview source names.
     */
    const QStringList& GetSourcesNames() const;

    /**
     * Returns the preview source at the specified index
     */
    /*const */TexturePreviewSource* GetSource(unsigned int index) const;


signals:
    /**
     * Emitted whenever a source gets available or unavailable or otherwise changed.
     */
    void AvailableSourcesListChanged();

private slots:
    void OnSourceAvailable();
    void OnSourceUnavailable();

    void RebuildSourceNamesList();

private:
    TexturePreviewManager(QObject* parent);
    ~TexturePreviewManager();

    static TexturePreviewManager* instance;

    QList<TexturePreviewSource*> sources;
    QStringList sourcesNames;
};
