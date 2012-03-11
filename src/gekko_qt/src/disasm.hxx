#include <QDockWidget>
#include "ui_disasm.h"

class QStandardItemModel;

class GDisAsmView : public QDockWidget
{
    Q_OBJECT

public:
    GDisAsmView(QWidget* parent = NULL);

public slots:
    void OnUpdate();
    void OnSetBreakpoint();

private:
    Ui::DockWidget disasm_ui;
    QStandardItemModel* model;

//    QList<u32> breakpoints;
};
