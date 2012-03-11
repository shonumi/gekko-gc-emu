#include "gekko_regs.hxx"

#include "powerpc/cpu_core_regs.h"

GGekkoRegsView::GGekkoRegsView(QWidget* parent) : QDockWidget(parent)
{
    Ui::GekkoRegisters gekko_regs_ui;
    gekko_regs_ui.setupUi(this);

    tree = gekko_regs_ui.treeWidget;

    // TODO: memleaks?
    tree->addTopLevelItem(fpr = new QTreeWidgetItem(QStringList("fpr")));
    tree->addTopLevelItem(PC = new QTreeWidgetItem(QStringList("PC")));
    tree->addTopLevelItem(gpr = new QTreeWidgetItem(QStringList("gpr")));
    tree->addTopLevelItem(TBR = new QTreeWidgetItem(QStringList("TBR")));
    tree->addTopLevelItem(MSR = new QTreeWidgetItem(QStringList("MSR")));
    tree->addTopLevelItem(CR = new QTreeWidgetItem(QStringList("CR")));
    tree->addTopLevelItem(FPSCR = new QTreeWidgetItem(QStringList("FPSCR")));
    tree->addTopLevelItem(IC = new QTreeWidgetItem(QStringList("IC")));
    tree->addTopLevelItem(sr = new QTreeWidgetItem(QStringList("sr")));
    tree->addTopLevelItem(spr = new QTreeWidgetItem(QStringList("spr")));

    // TODO: The register name shouldn't be editable
    // TODO: Input validating
    const Qt::ItemFlags child_flags = Qt::ItemIsEditable | Qt::ItemIsEnabled;
    PC->setFlags(child_flags);
    TBR->setFlags(child_flags);
    MSR->setFlags(child_flags);
    CR->setFlags(child_flags);
    FPSCR->setFlags(child_flags);
    IC->setFlags(child_flags);
    for (int i = 0; i < 32; ++i)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(QString("fpr[%1]").arg(i)));
        child->setFlags(child_flags);
        fpr->addChild(child);
    }

    for (int i = 0; i < 33; ++i)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(QString("gpr[%1]").arg(i)));
        child->setFlags(child_flags);
        gpr->addChild(child);
    }

    for (int i = 0; i < 16; ++i)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(QString("sr[%1]").arg(i)));
        child->setFlags(child_flags);
        sr->addChild(child);
    }

    for (int i = 0; i < 1024; ++i)
    {
        QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(QString("spr[%1]").arg(i)));
        child->setFlags(child_flags);
        spr->addChild(child);
    }

    connect(gekko_regs_ui.button_update, SIGNAL(clicked()), this, SLOT(OnUpdate()));
}

void GGekkoRegsView::OnUpdate()
{
    // TODO: not thread safe...

    // TODO: Order correct?
//    for (int i = 0; i < 32; ++i)
//        fpr->child(i)->setText(1, QString("0x%1%2").arg(ireg.fpr[i].ps0, 16, 16, QLatin1Char('0')).arg(ireg.fpr[i].ps1, 16, 16, QLatin1Char('0')));

    PC->setText(1, QString("0x%1").arg(ireg.PC, 8, 16, QLatin1Char('0')));

    for (int i = 0; i < 33; ++i)
        gpr->child(i)->setText(1, QString("0x%1").arg(ireg.gpr[i], 8, 16, QLatin1Char('0')));


    TBR->setText(1, QString("0x%1").arg(ireg.TBR.TBR, 16, 16, QLatin1Char('0'))); // TODO: Order correct?
    MSR->setText(1, QString("0x%1").arg(ireg.MSR, 8, 16, QLatin1Char('0')));
    CR->setText(1, QString("0x%1").arg(ireg.CR, 8, 16, QLatin1Char('0')));
    FPSCR->setText(1, QString("0x%1").arg(ireg.FPSCR, 8, 16, QLatin1Char('0')));
    IC->setText(1, QString("0x%1").arg(ireg.IC, 8, 16, QLatin1Char('0')));

    for (int i = 0; i < 16; ++i)
        sr->child(i)->setText(1, QString("0x%1").arg(ireg.sr[i], 8, 16, QLatin1Char('0')));

    for (int i = 0; i < 1024; ++i)
        spr->child(i)->setText(1, QString("0x%1").arg(ireg.spr[i], 8, 16, QLatin1Char('0')));
}
