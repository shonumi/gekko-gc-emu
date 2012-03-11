#include <QtGui>
#include "ui_disasm.h"
#include "disasm.hxx"

#include "common.h"
#include "powerpc/cpu_core_regs.h"
#include "memory.h"
#include "powerpc/disassembler/ppc_disasm.h"

GDisAsmView::GDisAsmView(QWidget* parent) : QDockWidget(parent)
{
    disasm_ui.setupUi(this);

    model = new QStandardItemModel(this);
    model->setRowCount(1000);
    model->setColumnCount(3);
    disasm_ui.treeView->setModel(model);

    connect(disasm_ui.pushButton_2, SIGNAL(clicked()), this, SLOT(OnUpdate()));
//    connect(disasm_ui.button_breakpoint, SIGNAL(clicked()), this, SLOT(OnSetBreakpoint()));
}

void GDisAsmView::OnUpdate()
{
    unsigned int curInstAddr = ireg.PC - 500;
    int  counter = 0;
    while(true)
    {
        u32 opcode = Memory_Read32(curInstAddr);

        char out1[64];
        char out2[128];
        u32 out3;
        DisassembleGekko(out1, out2, opcode, curInstAddr, &out3);
        // TODO: Leaking memory? :/
        model->setItem(counter, 0, new QStandardItem(QString("0x%1").arg((uint)curInstAddr, 8, 16, QLatin1Char('0'))));
        model->setItem(counter, 1, new QStandardItem(QString(out1)));
        model->setItem(counter, 2, new QStandardItem(QString(out2)));
        if (curInstAddr == ireg.PC) model->item(counter, 0)->setBackground(Qt::yellow);
        if (curInstAddr == ireg.PC) model->item(counter, 1)->setBackground(Qt::yellow);
        if (curInstAddr == ireg.PC) model->item(counter, 2)->setBackground(Qt::yellow);
        curInstAddr += 4;

        ++counter;
        if (counter >= 1000) break;
    }
    disasm_ui.treeView->resizeColumnToContents(0);
    disasm_ui.treeView->resizeColumnToContents(1);
    disasm_ui.treeView->resizeColumnToContents(2);
}

void GDisAsmView::OnSetBreakpoint()
{
/*    if (disasm_ui.treeView->selectedIndexes().size() == 0)
        return;

    // TODO: How to get the selected address correctly?
    u32 addr = 0;
    int index = breakpoints.indexOf(addr);
    if (index == -1) // no match
    {
        breakpoints.push_back(addr);
    }
    else
    {
        // TODO: Rename button..
        breakpoints.removeAt(index);
    }*/
}
