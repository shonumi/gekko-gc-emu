#include <QDockWidget>
#include "ui_disasm.h"

//#include "common.h"
typedef unsigned int u32;
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
    int SelectedRow();
    u32 SelectedAddress();

    Ui::DockWidget disasm_ui;
    QStandardItemModel* model;

    u32 base_addr;
//    QList<u32> breakpoints;
};
