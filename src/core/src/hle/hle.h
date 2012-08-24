// highlevel.h
// (c) 2005,2006 Gekko Team

#ifndef _HIGHLEVEL_H_
#define _HIGHLEVEL_H_

#pragma warning(disable:4786)

////////////////////////////////////////////////////////////

#define HLETYPE						void __cdecl
#define HLE(name)					HLETYPE HLE_##name(void)
#define HLE_PTR(name)				HLE_##name
#define HLE_FUNCTION(name)			HLE_##name()

#define HLE_PARAM_INT_0				(ireg.gpr[3])
#define HLE_PARAM_INT_1				(ireg.gpr[4])
#define HLE_PARAM_INT_2				(ireg.gpr[5])

#define HLE_PARAM_FLOAT_0			(ireg.fpr[1].ps0._f64)
#define HLE_PARAM_FLOAT_1			(ireg.fpr[2].ps0._f64)
#define HLE_PARAM_FLOAT_2			(ireg.fpr[3].ps0._f64)

#define HLE_PARAM_PTR_0				&RAM[HLE_PARAM_INT_0 & RAM_MASK]
#define HLE_PARAM_PTR_1				&RAM[HLE_PARAM_INT_1 & RAM_MASK]
#define HLE_PARAM_PTR_2				&RAM[HLE_PARAM_INT_2 & RAM_MASK]

#define HLE_RETURN(X)				ireg.gpr[3] = (X)
#define HLE_RETURN_TRUE				ireg.gpr[3] = 1
#define HLE_RETURN_FALSE			ireg.gpr[3] = 0

#define sd2							(ireg.gpr[13])
#define OSRoundUp32B(x)				(((u32)(x) + 32 - 1) & ~(32 - 1))

////////////////////////////////////////////////////////////

#define OS_DATA_HI 0
#define OS_DATA_LO 1
#define OS_DATA_32 2

#define OS_SCAN_BLR 0
#define OS_SCAN_RFI 1

#define OS_SCAN_HW_CP				0x0000
#define OS_SCAN_HW_PE				0x1000
#define OS_SCAN_HW_VI				0x2000
#define OS_SCAN_HW_PI				0x3000
#define OS_SCAN_HW_MI				0x4000
#define OS_SCAN_HW_DSP				0x5000
#define OS_SCAN_HW_DI				0x6000
#define OS_SCAN_HW_SI				0x6400
#define OS_SCAN_HW_EXI				0x6800
#define OS_SCAN_HW_AI				0x6C00
#define OS_SCAN_HW_GX				0x8000

#define OS_START_ADDR(x)			(hle_ranges[x][0])
#define OS_END_ADDR(x)				(hle_ranges[x][1])

typedef struct _OSCallFormat
{
	DWORD	Vals[13];
} OSCallFormat;

extern OSCallFormat oscall[900];
extern bool DisableHLEPatches;

#define MAX_HLE_FUNCTIONS 0x10000

typedef void (*HLEFuncPtr)(void);

extern HLEFuncPtr g_hle_func_table[MAX_HLE_FUNCTIONS];
extern u16 g_hle_count;

////////////////////////////////////////////////////////////

struct Function
{
	uintptr_t address;
    u32 funcSize;
	u32 DetectedSize;
	u32 CRC;
	std::string funcName;
	std::string fileName;    
};

typedef struct t_mapFile
{
	char path[255];
	char title[255];
}mapFile;

extern std::map<u32, Function> maps;
extern std::map<u32, u32> mapsCRCAddress;

////////////////////////////////////////////////////////////

typedef void(*hle_functions)();
extern hle_functions exec_func_HLE[80];

////////////////////////////////////////////////////////////

void HLE_GetGameCRC(char *gameCRC, u8 *Header, u8 BannerCRC);
void HLE_ScanForPatches(void);
void HLE_ExecuteLowLevel(void);
void HLE_Map_OpenFile(void);
void HLE_Map2Crc(void);
void HLE_MapSetDebugSymbol(u32 add, std::string name);
bool HLE_MapGetDebugSymbol(u32 add, std::string& name);
bool HLE_MapGetDebugSymbolFunctionAddress(const std::string& _name, u32& _addr);
void HLE_FindFuncsAndGenerateCRCs();
u32 HLE_DetectFunctionSize(u32 addr);
u32 HLE_GenerateFunctionCRC(u32 Addr, u32 FuncSize);

////////////////////////////////////////////////////////////

#endif