#ifndef _CONTROLLER_CONFIG_UTIL_HXX_
#define _CONTROLLER_CONFIG_UTIL_HXX_

#include <QWidget>
#include <QPushButton>

#include "config.h"

class GStickConfig : public QWidget
{
    Q_OBJECT

public:
    // Parent needs to have a OnKeyConfigChanged(common::Config::Control, int, const QString&) slot!
    GStickConfig(common::Config::Control leftid, common::Config::Control rightid, common::Config::Control upid, common::Config::Control downid, QWidget* parent = NULL);

signals:
    void LeftChanged();
    void RightChanged();
    void UpChanged();
    void DownChanged();

    void KeyConfigChanged(common::Config::Control, int key, const QString&);

public slots:
    void OnKeyConfigChanged(common::Config::Control, int key, const QString&);

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
    // Parent needs to have a OnKeyConfigChanged(common::Config::Control, int, const QString&) slot!
    GKeyConfigButton(common::Config::Control id, const QIcon& icon, const QString& text, QWidget* parent);
    GKeyConfigButton(common::Config::Control id, const QString& text, QWidget* parent);

signals:
    void KeyAssigned(common::Config::Control id, int key, const QString& text);

private slots:
    void OnClicked();

    void keyPressEvent(QKeyEvent* event); // TODO: bGrabbed?
    void mousePressEvent(QMouseEvent* event);

private:
    common::Config::Control id;
    bool inputGrabbed;
};

class GButtonConfigGroup : public QWidget
{
public:
    GButtonConfigGroup(const QString& name, common::Config::Control id, QWidget* parent = NULL);

private:
    GKeyConfigButton* config_button;

    common::Config::Control id;
};

#endif  // _CONTROLLER_CONFIG_HXX_
