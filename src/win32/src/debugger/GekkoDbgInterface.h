
#pragma once
#include <vector>
using namespace std;

class GekkoDebugInterface : public DebugInterface
{
public:
	GekkoDebugInterface(){} 
	virtual char *disasm(unsigned int address);
	virtual int getInstructionSize(int instruction) {return 2;}
	virtual bool isBreakpoint(unsigned int address);
	virtual void setBreakpoint(unsigned int address);
	virtual void clearBreakpoint(unsigned int address);
	virtual void clearAllBreakpoints();
	virtual void toggleBreakpoint(unsigned int address);
	virtual unsigned int readMemory(unsigned int address);
	virtual unsigned int getPC();
	virtual void setPC(unsigned int address);
	virtual void step() {}
	virtual void runToBreakpoint();
	virtual int getColor(unsigned int address);
	virtual char *getDescription(unsigned int address);
};

extern GekkoDebugInterface di;

typedef struct
{
	u32 dwAddr;	// Address to break on
	u16 wFlags;	// 
	u16 wSaved;	// Saved Gekko Instruction (use this for ptr address on another arch)

} Bpt, BreakPoint;

extern vector<Bpt> bpt;
