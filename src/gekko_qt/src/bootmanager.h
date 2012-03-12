#include <QThread>
#include "common.h"

class EmuThread : public QThread
{
public:
	EmuThread(const char* filename);

	void run();

private:
	char filename[MAX_PATH];
};
