#include "qhexedit.h"
#include "platform.h"

class GRamView : public QHexEdit
{
    Q_OBJECT

public:
    GRamView(QWidget* parent = NULL);

    static void EMU_FASTCALL OnCPUStep(void* object);
};
