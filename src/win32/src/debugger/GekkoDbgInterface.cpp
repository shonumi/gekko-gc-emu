#include "emu.h"
#include "DebugInterface.h"
#include "GekkoDbgInterface.h"
#include <vector>
using namespace std;

vector<Bpt> bpt;

void DisplayError (char * Message, ...) {
	char Msg[1000];
	va_list ap;

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	MessageBox(NULL,(LPCWSTR)Msg,(LPCWSTR)"Error",MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
}

int Invector(u32 dwAddr, vector<Bpt> vec)
{
	for (u32 indx = 0; indx < vec.size(); indx++)
		if(vec[indx].dwAddr == dwAddr)
			return indx;
	return -1;
}

char szDisBuf[256];

char *GekkoDebugInterface::disasm(unsigned int address) 
{
	char opcodeStr[32], operandStr[32];
	u32 target;
	u32 opcode;  

    opcode = Memory_Read32(address);

	DisassembleGekko(opcodeStr, operandStr, opcode, address, &target);

	sprintf(szDisBuf, "%s\t%s", opcodeStr, operandStr);

	return szDisBuf;
}
unsigned int GekkoDebugInterface::readMemory(unsigned int address)
{
	if(0xFFFFFF00 == (address & 0xFFFFFF00))
		return 0xCCCDEB06;

	return Memory_Read32(address);
}

bool GekkoDebugInterface::isBreakpoint(unsigned int address) 
{
	return (Invector(address, bpt) != -1);
}

void GekkoDebugInterface::clearAllBreakpoints() {}

void GekkoDebugInterface::setBreakpoint(unsigned int address) {
	toggleBreakpoint(address);
}
void GekkoDebugInterface::clearBreakpoint(unsigned int address) {
	toggleBreakpoint(address);
}

#include <windows.h>
extern HWND hDebugger;
void RefreshDebugger(HWND hWnd);

void GekkoDebugInterface::toggleBreakpoint(unsigned int address)
{
	Bpt	Newbpt;

	int i = Invector(address, bpt);
	if(i == -1)
	{
		Newbpt.dwAddr = address;
		bpt.push_back(Newbpt);
	}
	else
	{
		bpt.erase(bpt.begin() + i);
	}
}

int curcol=0;
int GekkoDebugInterface::getColor(unsigned int address)
{
	int colors[7] = {0xe0FFFF,0xFFe0e0,0xe8e8FF,0xFFe0FF,0xe0FFe0,0xFFFFe0,0xAeE0FF};

	return colors[curcol&3];
}

////////////////////
////////////////////

extern u8 symtab[];
extern u8 strtab[];

extern u32 symindex;
extern bool bElfLoaded;

static char szDesc[256];

char *GekkoDebugInterface::getDescription(unsigned int address) 
{
	u32 dwOp = Memory_Read32(address);
	sprintf(szDesc, "0x%08X", dwOp);

	return szDesc;
}

unsigned int GekkoDebugInterface::getPC() 
{
	return ireg_PC();
}

void GekkoDebugInterface::setPC(unsigned int address) 
{
	set_ireg_PC(address);
}

void GekkoDebugInterface::runToBreakpoint() 
{
}