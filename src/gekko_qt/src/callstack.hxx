#include <QDockWidget>
#include "ui_callstack.h"

class QStandardItemModel;

class GCallstackView : public QDockWidget
{
    Q_OBJECT

public:
    GCallstackView(QWidget* parent = 0);

    static void OnCPUStep(void* object);

private:
    Ui::CallStack ui;
    QStandardItemModel* callstack_model;
};
