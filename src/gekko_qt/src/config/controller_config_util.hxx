#ifndef _CONTROLLER_CONFIG_UTIL_HXX_
#define _CONTROLLER_CONFIG_UTIL_HXX_

#include <QWidget>
#include <QPushButton>

enum ControllerButtonId
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_Z,
    TRIGGER_L,
    TRIGGER_R,
    BUTTON_START,
    MAINSTICK_LEFT,
    MAINSTICK_RIGHT,
    MAINSTICK_UP,
    MAINSTICK_DOWN,
    CSTICK_LEFT,
    CSTICK_RIGHT,
    CSTICK_UP,
    CSTICK_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    DPAD_UP,
    DPAD_DOWN,
};

class GStickConfig : public QWidget
{
    Q_OBJECT

public:
    // Parent needs to have a OnKeyConfigChanged(ControllerButtonId, int, const QString&) slot!
    GStickConfig(ControllerButtonId leftid, ControllerButtonId rightid, ControllerButtonId upid, ControllerButtonId downid, QWidget* parent = NULL);

signals:
    void LeftChanged();
    void RightChanged();
    void UpChanged();
    void DownChanged();

    void KeyConfigChanged(ControllerButtonId, int key, const QString&);

public slots:
    void OnKeyConfigChanged(ControllerButtonId, int key, const QString&);

private:
    QPushButton* left;
    QPushButton* right;
    QPushButton* up;
    QPushButton* down;

    QPushButton* clear;
};

class GKeyConfigButton : public QPushButton
{
    Q_OBJECT

public:
    // Parent needs to have a OnKeyConfigChanged(ControllerButtonId, int, const QString&) slot!
    GKeyConfigButton(ControllerButtonId id, const QIcon& icon, const QString& text, QWidget* parent);
    GKeyConfigButton(ControllerButtonId id, const QString& text, QWidget* parent);

signals:
    void KeyAssigned(ControllerButtonId id, int key, const QString& text);

private slots:
    void OnClicked();

    void keyPressEvent(QKeyEvent* event); // TODO: bGrabbed?
    void mousePressEvent(QMouseEvent* event);

private:
    ControllerButtonId id;
    bool inputGrabbed;
};

class GButtonConfigGroup : public QWidget
{
public:
    GButtonConfigGroup(const QString& name, ControllerButtonId id, QWidget* parent = NULL);

private:
    GKeyConfigButton* config_button;

    ControllerButtonId id;
};

#endif  // _CONTROLLER_CONFIG_HXX_
