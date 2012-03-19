#include <QKeySequence>
#include <QSettings>
#include "hotkeys.h"
#include <map>

namespace Hotkeys
{

struct Hotkey
{
    Hotkey() : shortcut(NULL), context(Qt::WindowShortcut) {}

    QKeySequence keyseq;
    QShortcut* shortcut;
    Qt::ShortcutContext context;
};

std::map<HotkeyAction, Hotkey> hotkeys;

const char* action_names[] = {
    "StartGame",
    "Disasm_Step",
//    "Disasm_StepInto",
//    "Disasm_Pause",
    "Disasm_Cont",
    "Disasm_Breakpoint",
};

void SaveHotkeys(QSettings& settings)
{
    settings.beginGroup("Shortcuts");
    for (int action = 0; action < NumActions; ++action)
    {
        settings.setValue(QString("ShortcutKeySeq_") + QString(action_names[action]), hotkeys[(HotkeyAction)action].keyseq.toString());
        settings.setValue(QString("ShortcutContext_") + QString(action_names[action]), hotkeys[(HotkeyAction)action].context);
    }
    settings.endGroup();
}

void LoadHotkeys(QSettings& settings)
{
    std::map<HotkeyAction, QString> default_keys;
    default_keys[StartGame] = "";
    default_keys[Disasm_Step] = "F10";
//    default_keys[Disasm_StepInto] = "F11";
//    default_keys[Disasm_Pause] = "F5";
    default_keys[Disasm_Cont] = "F5";
    default_keys[Disasm_Breakpoint] = "F9";

    std::map<HotkeyAction, Qt::ShortcutContext> default_contexts;
    default_contexts[StartGame] = Qt::WindowShortcut;
    default_contexts[Disasm_Step] = Qt::ApplicationShortcut;
//    default_contexts[Disasm_StepInto] = Qt::ApplicationShortcut;
//    default_contexts[Disasm_Pause] = Qt::ApplicationShortcut;
    default_contexts[Disasm_Cont] = Qt::ApplicationShortcut;
    default_contexts[Disasm_Breakpoint] = Qt::ApplicationShortcut;
    
    settings.beginGroup("Shortcuts");
    for (int action = 0; action < NumActions; ++action)
    {
        QString str = settings.value(QString("ShortcutKeySeq_") + QString(action_names[action]), default_keys[(HotkeyAction)action]).toString();
        hotkeys[(HotkeyAction)action].keyseq = QKeySequence::fromString(str);
        hotkeys[(HotkeyAction)action].context = (Qt::ShortcutContext)settings.value(QString("ShortcutContext_") + QString(action_names[action]), default_contexts[(HotkeyAction)action]).toInt();
    }

    settings.endGroup();
}

void SetHotkey(HotkeyAction action, const QKeySequence& keyseq, Qt::ShortcutContext context)
{
    hotkeys[action].keyseq = keyseq;
    hotkeys[action].context = context;

    if (hotkeys[action].shortcut)
        hotkeys[action].shortcut->setKey(keyseq);
}

QShortcut* GetShortcut(HotkeyAction action, QWidget* widget)
{
    Hotkey& hk = hotkeys[action];

    if (!hk.shortcut)
        hk.shortcut = new QShortcut(hk.keyseq, widget, NULL, NULL, hk.context);

    return hk.shortcut;
}

} // namespace Hotkeys
