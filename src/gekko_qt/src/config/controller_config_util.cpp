#include <QPushButton>
#include <QStyle>
#include <QGridLayout>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QLabel>

#include "controller_config_util.hxx"

GStickConfig::GStickConfig(common::Config::Control leftid, common::Config::Control rightid, common::Config::Control upid, common::Config::Control downid, QWidget* parent) : QWidget(parent)
{
    left = new GKeyConfigButton(leftid, style()->standardIcon(QStyle::SP_ArrowLeft), QString(), this);
    right = new GKeyConfigButton(rightid, style()->standardIcon(QStyle::SP_ArrowRight), QString(), this);
    up = new GKeyConfigButton(upid, style()->standardIcon(QStyle::SP_ArrowUp), QString(), this);
    down = new GKeyConfigButton(downid, style()->standardIcon(QStyle::SP_ArrowDown), QString(), this);
    clear = new QPushButton(tr("Clear"), this);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(left, 1, 0);
    layout->addWidget(right, 1, 2);
    layout->addWidget(up, 0, 1);
    layout->addWidget(down, 2, 1);
    layout->addWidget(clear, 1, 1);

    setLayout(layout);

    connect(this, SIGNAL(KeyConfigChanged(common::Config::Control, int, const QString&)), parent, SLOT(OnKeyConfigChanged(common::Config::Control, int, const QString&)));
}

void GStickConfig::OnKeyConfigChanged(common::Config::Control id, int key, const QString& name)
{
    emit KeyConfigChanged(id, key, name);
}

GKeyConfigButton::GKeyConfigButton(common::Config::Control id, const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(icon, text, parent), id(id), inputGrabbed(false)
{
    connect(this, SIGNAL(clicked()), this, SLOT(OnClicked()));
    connect(this, SIGNAL(KeyAssigned(common::Config::Control, int, const QString&)), parent, SLOT(OnKeyConfigChanged(common::Config::Control, int, const QString&)));
}

GKeyConfigButton::GKeyConfigButton(common::Config::Control id, const QString& text, QWidget* parent) : QPushButton(text, parent), id(id), inputGrabbed(false)
{
    connect(this, SIGNAL(clicked()), this, SLOT(OnClicked()));
    connect(this, SIGNAL(KeyAssigned(common::Config::Control, int, const QString&)), parent, SLOT(OnKeyConfigChanged(common::Config::Control, int, const QString&)));
}

void GKeyConfigButton::OnClicked()
{
    grabKeyboard();
    grabMouse();
    inputGrabbed = true;

    setText(tr("Input..."));
}

void GKeyConfigButton::keyPressEvent(QKeyEvent* event)
{
    if (inputGrabbed)
    {
        releaseKeyboard();
        releaseMouse();
        setText(QString());

        // TODO: Doesn't capture "return" key
        QString text = QKeySequence(event->key()).toString(); // has a nicer format
        if (event->modifiers() == Qt::ShiftModifier) text = tr("Shift");
        else if (event->modifiers() == Qt::ControlModifier) text = tr("Ctrl");
        else if (event->modifiers() == Qt::AltModifier) text = tr("Alt");
        else if (event->modifiers() == Qt::MetaModifier) text = tr("Meta");

        setText(text);
        emit KeyAssigned(id, event->key(), text);

        inputGrabbed = false;
    }

    QPushButton::keyPressEvent(event); // TODO: Necessary?
}

void GKeyConfigButton::mousePressEvent(QMouseEvent* event)
{
    // Abort key assignment
    if (inputGrabbed)
    {
        releaseKeyboard();
        releaseMouse();
        setText(QString());
        inputGrabbed = false;
    }

    QAbstractButton::mousePressEvent(event);
}

GButtonConfigGroup::GButtonConfigGroup(const QString& name, common::Config::Control id, QWidget* parent) : QWidget(parent), id(id)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    QPushButton* clear_button = new QPushButton(tr("Clear"));

    layout->addWidget(new QLabel(name, this));
    layout->addWidget(config_button = new GKeyConfigButton(id, QString(), this));
    layout->addWidget(clear_button);

    // TODO: connect config_button, clear_button

    setLayout(layout);
}
