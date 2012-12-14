#include <QDockWidget>
#include "ui_gfx_texture_preview.h"

class TexturePreviewWidget : public QDockWidget
{
    Q_OBJECT

public:
    TexturePreviewWidget(QWidget* parent = NULL);

public slots:
    void OnAvailableSourcesListChanged();

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
    TexturePreviewSource(const QString& name);

    virtual ~TexturePreviewSource();

    virtual const char* GetData() = 0;
    virtual unsigned int GetFormat() = 0; // TODO: Change to gp::TextureFormat
    virtual unsigned int GetWidth() = 0;
    virtual unsigned int GetHeight() = 0;

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
    void TextureUnvailable();
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
    const QStringList GetAvailableSourcesList();

signals:
    /**
     * Emitted whenever a source gets available or unavailable or otherwise changed.
     */
    void AvailableSourcesListChanged();

private slots:
    void OnSourceAvailable();
    void OnSourceUnavailable();

private:
    ~TexturePreviewManager();
    TexturePreviewManager(QObject* parent) : QObject(parent) {}

    static TexturePreviewManager* instance;
};
