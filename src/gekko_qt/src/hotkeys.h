#include <QShortcut>
#include <string>

class QKeySequence;

namespace Hotkeys
{

enum HotkeyAction {
    StartGame,
    Disasm_Step,
//    Disasm_StepInto,
//    Disasm_Pause,
    Disasm_Cont,
    Disasm_Breakpoint,
    NumActions,
};

void SetHotkey(HotkeyAction action, const QKeySequence& keyseq, Qt::ShortcutContext context = Qt::WindowShortcut);
QShortcut* GetShortcut(HotkeyAction action, QWidget* widget);

void SaveHotkeys(QSettings& settings);
void LoadHotkeys(QSettings& settings);

} // namespace Hotkeys
