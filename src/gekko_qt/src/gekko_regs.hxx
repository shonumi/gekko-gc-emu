#include <QDockWidget>
#include "ui_gekko_regs.h"

class QTreeWidget;

class GGekkoRegsView : public QDockWidget
{
    Q_OBJECT

public:
    GGekkoRegsView(QWidget* parent = NULL);

public slots:
    void OnCPUStepped();

private:
    QTreeWidget* tree;
    QTreeWidgetItem* fpr;
    QTreeWidgetItem* PC;
    QTreeWidgetItem* gpr;
    QTreeWidgetItem* TBR;
    QTreeWidgetItem* MSR;
    QTreeWidgetItem* CR;
    QTreeWidgetItem* FPSCR;
    QTreeWidgetItem* IC;
    QTreeWidgetItem* sr;
    QTreeWidgetItem* spr;
};
