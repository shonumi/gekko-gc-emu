#include "ramview.hxx"

#include "common.h"
#include "memory.h"
#include "debugger/debugger.h"

GRamView::GRamView(QWidget* parent) : QHexEdit(parent)
{
    Debugger::RegisterCPUStepCallback(GRamView::OnCPUStep, this);
}

void GRamView::OnCPUStep(void* object)
{
    // TODO: Not thread save, will cause crashes..
    GRamView* obj = reinterpret_cast<GRamView*>(object);
    obj->setData(QByteArray((const char*)Mem_RAM,sizeof(Mem_RAM)/8)); // TODO: QHexEdit doesn't show vertical scroll bars for > 10MB data streams...
}
