#include <QThread>
#include "common.h"

class EmuThread : public QThread
{
    Q_OBJECT

public:
    static void Init();
    static EmuThread* GetInstance();
    static void Shutdown();

    void SetFilename(const char* filename);
	void run();

    void ExecStep() { exec_cpu_step = true; }
    void SetCpuRunning(bool running) { cpu_running = running; }

private:
	EmuThread();

	char filename[MAX_PATH];

    bool exec_cpu_step;
    bool cpu_running;

signals:
    void CPUStepped();
};

