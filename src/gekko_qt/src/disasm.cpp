#include <QtGui>
#include "ui_disasm.h"
#include "disasm.hxx"

#include "bootmanager.hxx"

#include "common.h"
#include "memory.h"
#include "debugger/debugger.h"
#include "powerpc/cpu_core_regs.h"
#include "powerpc/disassembler/ppc_disasm.h"

#include "powerpc/interpreter/cpu_int.h"

GDisAsmView::GDisAsmView(QWidget* parent) : QDockWidget(parent), base_addr(0)
{
    disasm_ui.setupUi(this);

    model = new QStandardItemModel(this);
    model->setColumnCount(3);
    disasm_ui.treeView->setModel(model);

    connect(disasm_ui.button_breakpoint, SIGNAL(clicked()), this, SLOT(OnSetBreakpoint()));
    connect(disasm_ui.button_step, SIGNAL(clicked()), this, SLOT(OnStep()));
    connect(disasm_ui.button_pause, SIGNAL(clicked()), this, SLOT(OnPause()));
    connect(disasm_ui.button_continue, SIGNAL(clicked()), this, SLOT(OnContinue()));
}

void GDisAsmView::OnSetBreakpoint()
{
    if (Debugger::IsBreakpoint(SelectedAddress()))
    {
        Debugger::UnsetBreakpoint(SelectedAddress());
        model->item(SelectedRow(), 0)->setBackground(QBrush());
        model->item(SelectedRow(), 1)->setBackground(QBrush());
        model->item(SelectedRow(), 2)->setBackground(QBrush());
    }
    else
    {
        Debugger::SetBreakpoint(SelectedAddress());
        model->item(SelectedRow(), 0)->setBackground(Qt::red);
        model->item(SelectedRow(), 1)->setBackground(Qt::red);
        model->item(SelectedRow(), 2)->setBackground(Qt::red);
    }
}

void GDisAsmView::OnStep()
{
    EmuThread::GetInstance()->SetCpuRunning(false);
    EmuThread::GetInstance()->ExecStep();
}

void GDisAsmView::OnPause()
{
    EmuThread::GetInstance()->SetCpuRunning(false);
}

void GDisAsmView::OnContinue()
{
    EmuThread::GetInstance()->SetCpuRunning(true);
}

void GDisAsmView::OnCPUStepped()
{
    base_addr = PTR_PC - 52;
    unsigned int curInstAddr = base_addr;
    int  counter = 0;
    QModelIndex cur_instr_index;
    model->setRowCount(100);
    while(true)
    {
        u32 opcode = Memory_Read32(curInstAddr);

        char out1[64];
        char out2[128];
        u32 out3 = 0;
        memset(out1, 0, sizeof(out1));
        memset(out2, 0, sizeof(out2));
        // TODO: Fix for linux
#if EMU_PLATFORM == PLATFORM_WINDOWS
        DisassembleGekko(out1, out2, opcode, curInstAddr, &out3);
#endif
        model->setItem(counter, 0, new QStandardItem(QString("0x%1").arg((uint)curInstAddr, 8, 16, QLatin1Char('0'))));
        model->setItem(counter, 1, new QStandardItem(QString(out1)));
        model->setItem(counter, 2, new QStandardItem(QString(out2)));

        if (curInstAddr == PTR_PC)
        {
            model->item(counter, 0)->setBackground(Qt::yellow);
            model->item(counter, 1)->setBackground(Qt::yellow);
            model->item(counter, 2)->setBackground(Qt::yellow);
            cur_instr_index = model->index(counter, 0);
        }
        else
        {
            model->item(counter, 0)->setBackground(QBrush());
            model->item(counter, 1)->setBackground(QBrush());
            model->item(counter, 2)->setBackground(QBrush());
        }
        curInstAddr += 4;

        ++counter;
        if (counter >= 100) break;
    }
    disasm_ui.treeView->resizeColumnToContents(0);
    disasm_ui.treeView->resizeColumnToContents(1);
    disasm_ui.treeView->resizeColumnToContents(2);
    disasm_ui.treeView->scrollTo(cur_instr_index); // QAbstractItemView::PositionAtCenter?
}

int GDisAsmView::SelectedRow()
{
    return model->itemFromIndex(disasm_ui.treeView->selectionModel()->currentIndex())->row();
}

u32 GDisAsmView::SelectedAddress()
{
    return base_addr + 4 * SelectedRow();
}
