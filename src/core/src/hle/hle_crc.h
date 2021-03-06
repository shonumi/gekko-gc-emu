
#include "crc.h"

struct HLEData
{
	const char* 	FuncName;
	u32		FuncSize;
	u32		FuncHash;
};

HLEData HLE_CRCs[] =
{
{"OSReport",0x00000080,0x1A446162},
{"OSReport",0x00000080,0x4F6A6158},
{"OSPanic",0x00000138,0xAD2C6116},
{"DSPAddTask",0x00000084,0xBD383605},
{"__DSPHandler",0x00000304,0x5ACE364B},
{"OnReset",0x00000010,0xAD3A7A2E},
{"TypeAndStatusCallback",0x00000118,0x73B79EB1},
{"PPCMfmsr",0x00000008,0x509ACF7F},
{"PPCMtmsr",0x00000008,0x72FDD29A},
{"PPCMfhid0",0x00000008,0x2BF2D9FC},
{"PPCMthid0",0x00000008,0xA4545767},
{"PPCMfl2cr",0x00000008,0xA151F21C},
{"PPCMtl2cr",0x00000008,0x2EF77C87},
{"PPCMtdec",0x00000008,0x538B6A04},
{"PPCSync",0x00000008,0x03FF7106},
{"PPCMtfpscr",0x00000028,0x40573D17},
{"PPCMfhid2",0x00000008,0x6F301683},
{"PPCMthid2",0x00000008,0xE0969818},
{"PPCMtwpar",0x00000008,0xD85C7795},
{"OSGetConsoleType",0x00000028,0x67FEF48F},
{"ClearArena",0x00000128,0xDEB65ADC},
{"OSInit",0x00000378,0xB6F95C3B},
{"OSExceptionInit",0x00000280,0x9B08A3D2},
{"__OSGetExceptionHandler",0x00000014,0x71C48ED5},
{"OSDefaultExceptionHandler",0x00000058,0x2A9F85B3},
{"__OSPSInit",0x00000038,0x627AA39C},
{"__OSGetDIConfig",0x00000014,0xA984F940},
{"OSInitAlarm",0x0000004C,0x2E60A3A6},
{"OSCreateAlarm",0x00000010,0x6C33FFD3},
{"InsertAlarm",0x00000250,0x6F179EB1},
{"OSSetAlarm",0x00000068,0x62E4D9C6},
{"OSSetPeriodicAlarm",0x0000007C,0xF7F0B551},
{"OSCancelAlarm",0x0000011C,0x9DB495A3},
{"DecrementerExceptionHandler",0x00000050,0x5C5D8808},
{"DLInsert",0x000000AC,0xB3CA1B4C},
{"OSAllocFromHeap",0x000000FC,0x234DA8FA},
{"OSFreeToHeap",0x0000007C,0x69A20DBB},
{"OSSetCurrentHeap",0x00000010,0xE7C0D433},
{"OSInitAlloc",0x00000070,0x6D1DB3F8},
{"OSCreateHeap",0x0000006C,0x87F951BA},
{"OSDestroyHeap",0x00000014,0xAF6C6116},
{"OSCheckHeap",0x00000360,0x1C5C0000},
{"OSReferentSize",0x0000000C,0xDDD02086},
{"OSDumpHeap",0x00000104,0xFB19E91E},
{"OSGetArenaHi",0x00000008,0xB802D409},
{"OSGetArenaLo",0x00000008,0xB802D409},
{"OSSetArenaHi",0x00000008,0xC38E3BBE},
{"OSSetArenaLo",0x00000008,0xC38E3BBE},
{"OSAllocFromArenaLo",0x0000002C,0xBB900074},
{"__OSInitAudioSystem",0x000001BC,0xDFC2C28A},
{"__OSStopAudioSystem",0x000000D8,0x262C95D7},
{"DCEnable",0x00000014,0x14FB8832},
{"DCInvalidateRange",0x0000002C,0xD072A3E8},
{"DCFlushRange",0x00000030,0xB2909822},
{"DCStoreRange",0x00000030,0xB2909822},
{"DCFlushRangeNoSync",0x0000002C,0xD072A3E8},
{"DCStoreRangeNoSync",0x0000002C,0xD072A3E8},
{"DCZeroRange",0x0000002C,0xD072A3E8},
{"ICInvalidateRange",0x00000034,0xFDCF0B12},
{"ICFlashInvalidate",0x00000010,0x3F174AF6},
{"ICEnable",0x00000014,0x57B1E96A},
{"__LCEnable",0x000000CC,0x1D0A7A5A},
{"LCEnable",0x00000038,0x81719EFF},
{"LCDisable",0x00000028,0xE3D75C01},
{"LCStoreBlocks",0x00000024,0x6FFE95ED},
{"LCStoreData",0x000000AC,0xA2BDF21C},
{"LCQueueWait",0x00000018,0x1850CF45},
{"L2GlobalInvalidate",0x00000098,0x90671D91},
{"DMAErrorHandler",0x00000160,0x068D3D2D},
{"__OSCacheInit",0x000000F4,0x3150F4B5},
{"__OSLoadFPUContext",0x00000124,0xCD6DD2EE},
{"__OSSaveFPUContext",0x00000128,0x44BA6158},
{"OSSaveFPUContext",0x00000008,0x06BE1B38},
{"OSSetCurrentContext",0x0000005C,0x40A76A04},
{"OSGetCurrentContext",0x0000000C,0xD8DD1DAB},
{"OSSaveContext",0x00000080,0x6BA23BCA},
{"OSLoadContext",0x000000D8,0x12E27A5A},
{"OSGetStackPointer",0x00000008,0x823530D8},
{"OSClearContext",0x00000024,0xE2FA363F},
{"OSInitContext",0x000000BC,0x4947C423},
{"OSDumpContext",0x000002A8,0xBC495C3B},
{"OSSwitchFPUContext",0x00000084,0x96880DF5},
{"__OSContextInit",0x00000048,0xD147A8B4},
{"OSFillFPUContext",0x0000012C,0x7E05E4EB},
{"OSSetErrorHandler",0x00000218,0x23146CE3},
{"__OSUnhandledException",0x000002E8,0x7FDEA39C},
{"OSGetFontEncode",0x00000058,0x8A1D30D8},
{"OSDisableInterrupts",0x00000014,0x2462EF8D},
{"OSEnableInterrupts",0x00000014,0x2462EF8D},
{"OSRestoreInterrupts",0x00000024,0x69A820F2},
{"__OSSetInterruptHandler",0x0000001C,0x2AE620C8},
{"__OSGetInterruptHandler",0x00000014,0x7D111DDF},
{"__OSInterruptInit",0x00000074,0x15B3E950},
{"SetInterruptMask",0x000002D8,0xA4809822},
{"__OSMaskInterrupts",0x00000088,0xAADDF21C},
{"__OSUnmaskInterrupts",0x00000088,0x024A836E},
{"__OSDispatchInterrupt",0x00000344,0xCF5B9EC5},
{"ExternalInterruptHandler",0x00000050,0x5C5D8808},
{"OSNotifyLink",0x00000004,0x5032CF7F},
{"OSNotifyUnlink",0x00000004,0x5032CF7F},
{"OSSetStringTable",0x0000000C,0x33E930AC},
{"Relocate",0x000002BC,0xE4042D07},
{"Link",0x000002E0,0x72904190},
{"OSLink",0x00000024,0xD1888EA1},
{"Undo",0x00000238,0xCCDFFFA7},
{"OSUnlink",0x000001D4,0xDEB7FFA7},
{"__OSModuleInit",0x00000018,0x0EA61B38},
{"OSInitMessageQueue",0x00000060,0xF781887C},
{"OSSendMessage",0x000000C8,0x7666F48F},
{"OSReceiveMessage",0x000000DC,0x696AF48F},
{"OSGetConsoleSimulatedMemSize",0x0000000C,0xD8DD1DAB},
{"OnReset",0x0000003C,0x4359105E},
{"MEMIntrruptHandler",0x0000006C,0xC5A816F7},
{"OSProtectRange",0x000000C4,0x9D69C9EC},
{"Config24MB",0x00000080,0x9501FF9D},
{"Config48MB",0x00000080,0x9501FF9D},
{"RealMode",0x00000018,0xA2509822},
{"__OSInitMemoryProtection",0x00000120,0x053D3D2D},
{"OSInitMutex",0x00000038,0x2CDF85B3},
{"OSLockMutex",0x000000DC,0x28D967BF},
{"OSUnlockMutex",0x000000C8,0x788077E1},
{"__OSUnlockAllMutex",0x00000070,0x14293D2D},
{"OSTryLockMutex",0x000000BC,0x2B35C998},
{"OSInitCond",0x00000020,0xB50AD409},
{"OSWaitCond",0x000000D4,0x0743DF21},
{"OSSignalCond",0x00000020,0xB50AD409},
{"__OSCheckMutex",0x00000100,0xC5A6F4FB},
{"__OSCheckDeadLock",0x00000038,0x0A6FE950},
{"__OSCheckMutexes",0x00000074,0x76D7A8C0},
{"Run",0x00000010,0xB0931010},
{"Callback",0x0000000C,0x52B82D73},
{"__OSReboot",0x000001C8,0x8665C9EC},
{"OSSetSaveRegion",0x0000000C,0xCAE185FD},
{"OSRegisterResetFunction",0x00000084,0xBA64F940},
{"Reset",0x00000070,0x7608D9C6},
{"__OSDoHotReset",0x00000048,0x49E02D73},
{"OSGetResetCode",0x00000030,0xED88B551},
{"__OSResetSWInterruptHandler",0x000000F4,0x232CF4B5},
{"OSGetResetButtonState",0x00000298,0x0A4A4C2B},
{"OSGetResetSwitchState",0x00000020,0xB50AD409},
{"WriteSramCallback",0x00000060,0x912E77AF},
{"WriteSram",0x00000118,0xB3343605},
{"__OSInitSram",0x00000134,0xDC557CC9},
{"__OSLockSram",0x0000005C,0xB8577172},
{"__OSLockSramEx",0x0000005C,0xF7A6004E},
{"UnlockSram",0x00000308,0xAA038846},
{"__OSUnlockSram",0x00000024,0xD1888EA1},
{"__OSUnlockSramEx",0x00000024,0xD1888EA1},
{"__OSSyncSram",0x00000010,0xAEE4CF31},
{"OSGetSoundMode",0x00000080,0x4BF3F252},
{"OSSetSoundMode",0x000000A4,0xB19A7A2E},
{"OSGetProgressiveMode",0x00000070,0xD22C2086},
{"OSSetProgressiveMode",0x000000A4,0x15DD6A3E},
{"OSGetWirelessID",0x00000084,0x6E2EC2FE},
{"OSSetWirelessID",0x000000AC,0x3882D9FC},
{"SystemCallVector",0x00000020,0x9DBC3B84},
{"__OSInitSystemCall",0x00000064,0xA15C3605},
{"OSInitThreadQueue",0x00000010,0x6C33FFD3},
{"OSGetCurrentThread",0x0000000C,0xD8DD1DAB},
{"OSIsThreadTerminated",0x00000034,0xFD930B12},
{"OSDisableScheduler",0x00000040,0xDF514AB8},
{"OSEnableScheduler",0x00000040,0xDF514AB8},
{"UnsetRun",0x00000068,0x5242F90E},
{"__OSGetEffectivePriority",0x0000003C,0x40A8B525},
{"SetEffectivePriority",0x000001C0,0x980530D8},
{"__OSPromoteThread",0x00000050,0xF9D54739},
{"__OSReschedule",0x00000030,0x51B42D73},
{"OSYieldThread",0x0000003C,0x2CE0C2C4},
{"OSCreateThread",0x000001E8,0xDD9C41E4},
{"OSExitThread",0x000000E4,0xC4E841E4},
{"OSCancelThread",0x000001BC,0x3B9951CE},
{"OSJoinThread",0x00000140,0x2054A3A6},
{"OSDetachThread",0x000000A0,0xA3116A4A},
{"OSResumeThread",0x00000288,0xF3A1E91E},
{"OSSuspendThread",0x00000170,0x4E9F9330},
{"OSSleepThread",0x000000EC,0xA206831A},
{"OSWakeupThread",0x00000104,0x5A3C4C11},
{"OSSetThreadPriority",0x000000C0,0x59AA6158},
{"OSGetThreadPriority",0x00000008,0x85799EFF},
{"CheckThreadQueue",0x0000009C,0x9B74C2B0},
{"OSCheckActiveThreads",0x00000750,0x7D99E4EB},
{"OSGetTime",0x00000018,0xC9CC41E4},
{"OSGetTick",0x00000008,0xDE93C9D6},
{"__OSGetSystemTime",0x00000064,0xD9B2C28A},
{"__OSTimeToSystemTime",0x00000058,0x8E78A3D2},
{"GetDates",0x0000019C,0x8DD05AE6},
{"OSTicksToCalendarTime",0x00000204,0x0F967A5A},
{"__init_user",0x00000020,0xB50AD409},
{"",0x00000010,0x75076785},
{"_ExitProcess",0x00000020,0xB50AD409},
{"SetExiInterruptMask",0x000000F4,0xAC115C3B},
{"EXIImm",0x0000025C,0x7978EFB7},
{"EXIImmEx",0x000000A0,0xB2FC0074},
{"EXIDma",0x000000EC,0x70F57CBD},
{"EXISync",0x00000238,0x8B3241DE},
{"EXIClearInterrupts",0x00000048,0x5211262F},
{"EXISetExiCallback",0x0000007C,0xEDC08320},
{"__EXIProbe",0x00000174,0x0BD67A5A},
{"EXIProbe",0x00000080,0x4CCECF7F},
{"EXIProbeEx",0x000000B4,0x46F0E236},
{"EXIAttach",0x0000010C,0x9830C2B0},
{"EXIDetach",0x000000BC,0x93C1A88E},
{"EXISelect",0x0000012C,0xACFF2661},
{"EXIDeselect",0x00000110,0xA812E278},
{"EXIIntrruptHandler",0x000000C8,0x83E6EFF9},
{"TCIntrruptHandler",0x00000218,0x743077E1},
{"EXTIntrruptHandler",0x000000D0,0x85D19EFF},
{"EXIInit",0x0000011C,0xF4B14739},
{"EXILock",0x000000F4,0x02FFBE43},
{"EXIUnlock",0x000000DC,0x2DB506DD},
{"EXIGetState",0x00000018,0xCA3AA3E8},
{"UnlockedHandler",0x00000028,0x92900DF5},
{"EXIGetID",0x0000037C,0x683CB8A4},
{"InitializeUART",0x00000070,0xBAA86116},
{"WriteUARTN",0x00000200,0xC32730E2},
{"SIBusy",0x00000020,0xBD82E278},
{"SIIsChanBusy",0x0000003C,0x10A05713},
{"CompleteTransfer",0x000002FC,0x918ED988},
{"SIInterruptHandler",0x00000344,0x9502EFF9},
{"SIEnablePollingInterrupt",0x00000098,0x7D1020F2},
{"SIRegisterPollingHandler",0x000000CC,0x5F6C4C11},
{"SIUnregisterPollingHandler",0x000000F4,0xAD821B4C},
{"SIInit",0x000000B4,0x55CE364B},
{"__SITransfer",0x0000020C,0xB28F8846},
{"SIGetStatus",0x0000007C,0x766E5AA8},
{"SISetCommand",0x00000014,0x59026158},
{"SITransferCommands",0x00000010,0xE7976A70},
{"SISetXY",0x0000006C,0xA0E34703},
{"SIEnablePolling",0x0000009C,0xF5C2AE69},
{"SIDisablePolling",0x0000006C,0xEBA35C01},
{"SIGetResponseRaw",0x000000D4,0x58C26158},
{"SIGetResponse",0x000000C4,0x843B4A82},
{"AlarmHandler",0x0000008C,0x1684CF45},
{"SITransfer",0x0000016C,0x1FFA2D49},
{"GetTypeCallback",0x00000298,0x837895A3},
{"SIGetType",0x000001C4,0x32A3E4D1},
{"SIGetTypeAsync",0x0000013C,0xA3FD0B28},
{"SIDecodeType",0x0000014C,0x395D67BF},
{"SIProbe",0x00000024,0x701A95ED},
{"SISetSamplingRate",0x000000E4,0xF4A81B76},
{"SIRefreshSamplingRate",0x00000024,0x701A95ED},
{"DBInit",0x00000028,0x6E4E3BCA},
{"__DBExceptionDestinationAux",0x00000048,0x6857C9A2},
{"__DBExceptionDestination",0x00000010,0xB745937E},
{"__DBIsExceptionMarked",0x0000001C,0xD3CB0693},
{"DBPrintf",0x00000050,0xB59B4703},
{"PSMTXIdentity",0x0000002C,0x19E2E20C},
{"PSMTXCopy",0x00000034,0xE5A1265B},
{"PSMTXConcat",0x000000CC,0x253530AC},
{"PSMTXInverse",0x000000F8,0x13BAD47D},
{"PSMTXRotRad",0x00000070,0x3F65C998},
{"PSMTXRotTrig",0x000000B0,0x91199EFF},
{"C_MTXRotAxisRad",0x00000118,0x8C53D2D4},
{"PSMTXRotAxisRad",0x00000070,0x3F65C998},
{"PSMTXTrans",0x00000034,0x72892BAE},
{"PSMTXTransApply",0x0000004C,0x8041FF9D},
{"PSMTXScale",0x00000028,0x03F13D2D},
{"PSMTXQuat",0x000000A4,0xE55B5C01},
{"C_MTXLookAt",0x0000018C,0x06F50B5C},
{"C_MTXLightPerspective",0x000000CC,0x1186B51F},
{"C_MTXLightOrtho",0x00000088,0x9E78F4C1},
{"PSMTXMultVec",0x00000054,0x9E959EFF},
{"PSMTXMultVecArray",0x0000008C,0x04196A3E},
{"PSMTXMultVecSR",0x00000054,0x8DD40DF5},
{"C_MTXPerspective",0x000000D0,0xBF66B56B},
{"C_MTXOrtho",0x00000098,0x5FF8B525},
{"PSVECAdd",0x00000024,0x4FC6364B},
{"PSVECSubtract",0x00000024,0x74191DDF},
{"PSVECScale",0x0000001C,0xFF870B12},
{"C_VECNormalize",0x000000C8,0xE9CA363F},
{"PSVECNormalize",0x00000044,0x43042D73},
{"PSVECSquareMag",0x00000018,0xF895102A},
{"PSVECMag",0x00000044,0xB30A4C5F},
{"PSVECDotProduct",0x00000020,0xE6970B12},
{"PSVECCrossProduct",0x0000003C,0x859EEFF9},
{"C_VECHalfAngle",0x000000D8,0x82B385C7},
{"C_VECReflect",0x000000D4,0x04F72615},
{"PSVECSquareDistance",0x00000028,0x7D31D29A},
{"PSVECDistance",0x00000054,0xF6E2363F},
{"PSQUATMultiply",0x0000005C,0x2653D2A0},
{"PSQUATNormalize",0x00000054,0xA04AD409},
{"PSQUATInverse",0x00000058,0x68BCEFB7},
{"C_QUATRotAxisRad",0x0000008C,0xEFACB551},
{"C_QUATSlerp",0x00000174,0xD6C60DCF},
{"__DVDInitWA",0x00000040,0xAD1C5767},
{"__DVDInterruptHandler",0x000002E0,0xA1572661},
{"AlarmHandler",0x00000084,0x5B4A5729},
{"AlarmHandlerForTimeout",0x00000070,0xBF9A831A},
{"Read",0x00000110,0x401C8354},
{"SeekTwiceBeforeRead",0x00000080,0x9F5785C7},
{"DVDLowRead",0x00000298,0x5403F252},
{"DVDLowSeek",0x00000094,0x2C85C998},
{"DVDLowWaitCoverClose",0x0000002C,0x2C8DC998},
{"DVDLowReadDiskID",0x000000A4,0xECFE9818},
{"DVDLowStopMotor",0x0000008C,0xD1451DAB},
{"DVDLowRequestError",0x0000008C,0xD1451DAB},
{"DVDLowInquiry",0x0000009C,0x6DB51DDF},
{"DVDLowAudioStream",0x00000098,0x74D51DDF},
{"DVDLowRequestAudioStatus",0x0000008C,0xD3C441E4},
{"DVDLowAudioBufferConfig",0x0000009C,0xB0DD5C3B},
{"DVDLowReset",0x000000BC,0x7E1F06E7},
{"DVDLowBreak",0x00000014,0x566F5C75},
{"DVDLowClearCallback",0x00000018,0xC62016F7},
{"__DVDLowSetWAType",0x00000044,0x506A364B},
{"__DVDFSInit",0x00000038,0xBCF9937E},
{"DVDConvertPathToEntrynum",0x000002F4,0x686C4190},
{"DVDFastOpen",0x00000074,0xBD945767},
{"DVDOpen",0x000000C8,0x239BD2A0},
{"DVDClose",0x00000024,0x952CF4C1},
{"entryToPath",0x00000160,0x53F42D73},
{"DVDConvertEntrynumToPath",0x00000154,0x3078F4B5},
{"DVDGetCurrentDir",0x000000C4,0x4E6A364B},
{"DVDChangeDir",0x00000060,0xE510D433},
{"DVDReadAsyncPrio",0x000000C0,0xA9510B28},
{"cbForReadAsync",0x00000030,0x15A9A57B},
{"DVDReadPrio",0x00000118,0x6539D29A},
{"cbForReadSync",0x00000024,0xD1888EA1},
{"DVDOpenDir",0x000000C0,0x65797CBD},
{"DVDReadDir",0x000000A4,0x53082D73},
{"DVDCloseDir",0x00000008,0x0D03BE43},
{"DVDPrepareStreamAsync",0x000000EC,0xAEC6831A},
{"cbForPrepareStreamAsync",0x00000030,0x15A9A57B},
{"DVDInit",0x000000CC,0x8BB5FF9D},
{"stateReadingFST",0x00000094,0x49179330},
{"cbForStateReadingFST",0x0000008C,0xBA846116},
{"cbForStateError",0x000000AC,0x3D1385B3},
{"stateTimeout",0x00000034,0xB72E4C5F},
{"stateGettingError",0x00000028,0x25EB4AF6},
{"CategorizeError",0x000000B4,0x545B9330},
{"cbForStateGettingError",0x00000294,0x2CFDC998},
{"cbForUnrecoveredError",0x00000068,0xA1872661},
{"cbForUnrecoveredErrorRetry",0x00000098,0x78E6F48F},
{"stateGoToRetry",0x00000028,0x25EB4AF6},
{"cbForStateGoToRetry",0x00000158,0xEC170B12},
{"stateCheckID",0x000000E0,0x3F831DE5},
{"stateCheckID3",0x00000034,0x50DE364B},
{"stateCheckID2",0x00000038,0xD2BBFFA7},
{"cbForStateCheckID1",0x00000114,0x4E6C7A60},
{"cbForStateCheckID2",0x000000E4,0xE1901B76},
{"cbForStateCheckID3",0x000000FC,0x0F346162},
{"AlarmHandler",0x00000044,0x2B17B3C2},
{"stateCoverClosed",0x000000CC,0x48C47A60},
{"stateCoverClosed_CMD",0x00000030,0x02303671},
{"cbForStateCoverClosed",0x00000070,0xD4560DCF},
{"stateMotorStopped",0x00000028,0x25EB4AF6},
{"cbForStateMotorStopped",0x000000E4,0xA8896A4A},
{"stateReady",0x000002DC,0x4DDD713C},
{"cbForStateBusy",0x00000638,0x17A6D47D},
{"DVDReadAbsAsyncPrio",0x000000DC,0x0CD78832},
{"DVDReadAbsAsyncForBS",0x000000D0,0x84677CF3},
{"DVDReadDiskID",0x000000D4,0x4A16CF7F},
{"DVDPrepareStreamAbsAsync",0x000000C4,0xBDA09822},
{"DVDCancelStreamAsync",0x000000BC,0x65E1D29A},
{"DVDStopStreamAtEndAsync",0x000000BC,0x65E1D29A},
{"DVDGetStreamPlayAddrAsync",0x000000BC,0x65E1D29A},
{"DVDInquiryAsync",0x000000D0,0x9DDB85C7},
{"DVDReset",0x00000044,0x4B21E96A},
{"DVDGetCommandBlockStatus",0x0000004C,0x719A3BCA},
{"DVDGetDriveStatus",0x000000AC,0x1EEE4C2B},
{"DVDSetAutoInvalidation",0x00000010,0xE7C0D433},
{"DVDCancelAsync",0x00000270,0x6890EFB7},
{"DVDCancel",0x000000AC,0xA1C2E278},
{"cbForCancelSync",0x00000024,0xD1888EA1},
{"DVDGetCurrentDiskID",0x00000008,0x5EA75C75},
{"DVDCheckDisk",0x000000E4,0xDCC985FD},
{"__DVDPrepareResetAsync",0x0000011C,0xC1DE5ADC},
{"__DVDClearWaitingQueue",0x00000038,0x9CB7D2D4},
{"__DVDPushWaitingQueue",0x00000068,0xEAF1E91E},
{"__DVDPopWaitingQueue",0x000000A0,0x2D2241AA},
{"__DVDCheckWaitingQueue",0x00000058,0x7207A8C0},
{"__DVDDequeueWaitingQueue",0x00000060,0xB18E7A2E},
{"ErrorCode2Num",0x0000011C,0x92EDFF9D},
{"__DVDStoreErrorCode",0x0000007C,0x3E0B1DE5},
{"cb",0x000000D8,0xF3A4B551},
{"__fstLoad",0x00000168,0xE31BF226},
{"__VIRetraceHandler",0x00000230,0xD6E47795},
{"VISetPreRetraceCallback",0x00000044,0xAF3CAE53},
{"VISetPostRetraceCallback",0x00000044,0xAF3CAE53},
{"getTiming",0x000000A0,0x7676A39C},
{"__VIInit",0x00000200,0x1963BE43},
{"VIInit",0x000004B0,0xFB46CF0B},
{"VIWaitForRetrace",0x00000054,0xF10F9344},
{"setFbbRegs",0x000002D4,0xB08AD409},
{"setVerticalRegs",0x000001A0,0x07E9C419},
{"VIConfigure",0x00000828,0xF101E91E},
{"VIFlush",0x00000130,0xA33F7172},
{"VISetNextFrameBuffer",0x0000006C,0x4F7B3D17},
{"VISetBlack",0x0000007C,0xF905DF6F},
{"VIGetRetraceCount",0x00000008,0xB802D409},
{"getCurrentFieldEvenOdd",0x00000068,0xAB2DC46D},
{"VIGetNextField",0x000000A8,0x7ABD2BAE},
{"VIGetCurrentLine",0x00000098,0x491A5729},
{"VIGetTvFormat",0x00000068,0x94B620BC},
{"VIGetDTVStatus",0x0000003C,0xE356363F},
{"ClampStick",0x00000130,0x5D5F5C75},
{"PADClamp",0x00000114,0xCD211DAB},
{"UpdateOrigin",0x000001A4,0xF5C6612C},
{"PADOriginCallback",0x000000C4,0x55EFC423},
{"PADOriginUpdateCallback",0x000000C0,0xF04C4C65},
{"PADProbeCallback",0x000000D8,0x0ADD0B5C},
{"PADTypeAndStatusCallback",0x0000032C,0x997ED988},
{"PADReceiveCheckCallback",0x00000134,0xCCE2A3E8},
{"PADReset",0x00000100,0x0E656A3E},
{"PADRecalibrate",0x00000104,0x121CCF45},
{"PADInit",0x00000218,0xC223A8B4},
{"PADRead",0x000003AC,0xF8348320},
{"PADControlMotor",0x000000A4,0x839DFF9D},
{"PADSetSpec",0x00000060,0x9636D988},
{"SPEC0_MakeStatus",0x00000174,0x6C47A8C0},
{"SPEC1_MakeStatus",0x00000174,0x60F14ACC},
{"SPEC2_MakeStatus",0x000003F8,0xF91FA535},
{"PADSetAnalogMode",0x00000074,0xD5E1E49F},
{"OnReset",0x00000194,0x87F5A88E},
{"SamplingHandler",0x00000060,0xFD0D4739},
{"PADSetSamplingCallback",0x00000054,0xA248F940},
{"__PADDisableRecalibration",0x0000007C,0x4C6D474D},
{"AIRegisterDMACallback",0x00000044,0xAF3CAE53},
{"AIInitDMA",0x00000088,0xEE3E612C},
{"AIStartDMA",0x00000018,0x9D032BE0},
{"AIResetStreamSampleCount",0x00000018,0x8B49C9EC},
{"AISetStreamPlayState",0x000000D8,0x89CF4A82},
{"AIGetStreamPlayState",0x00000010,0x46D65729},
{"AISetDSPSampleRate",0x000000E0,0xCF29B38C},
{"AIGetDSPSampleRate",0x00000014,0x60151DDF},
{"AISetStreamSampleRate",0x00000028,0x122ED47D},
{"__AI_set_stream_sample_rate",0x000000D4,0x21443BF0},
{"AIGetStreamSampleRate",0x00000010,0x3E1BE4D1},
{"AISetStreamVolLeft",0x0000001C,0x926D30D8},
{"AIGetStreamVolLeft",0x00000010,0xAB55C46D},
{"AISetStreamVolRight",0x0000001C,0x8FD06C97},
{"AIGetStreamVolRight",0x00000010,0xC21C41E4},
{"AIInit",0x0000016C,0x374A16B9},
{"__AISHandler",0x0000007C,0x370E41AA},
{"__AIDHandler",0x000000AC,0x21228EEF},
{"",0x00000020,0xAB305767},
{"__AI_SRC_INIT",0x000001E4,0x2F9220C8},
{"ARRegisterDMACallback",0x00000044,0xAF3CAE53},
{"ARStartDMA",0x000000F0,0xF1FA363F},
{"ARAlloc",0x00000068,0xB5F6D409},
{"ARInit",0x000000C4,0x88446C97},
{"ARGetBaseAddress",0x00000008,0x0D03BE43},
{"ARGetSize",0x00000008,0xB802D409},
{"__ARHandler",0x00000078,0x650E95ED},
{"__ARChecksize",0x000017F4,0x7A873096},
{"__ARQServiceQueueLo",0x00000100,0xB0B24C5F},
{"__ARQCallbackHack",0x00000004,0x5032CF7F},
{"__ARQInterruptServiceRoutine",0x000000CC,0xCCFB5180},
{"ARQInit",0x00000070,0x44BEAE1D},
{"ARQPostRequest",0x0000015C,0x0DBD0B5C},
{"DSPCheckMailToDSP",0x00000010,0x7928B8A4},
{"DSPCheckMailFromDSP",0x00000010,0x7928B8A4},
{"DSPReadMailFromDSP",0x00000018,0x38B530AC},
{"DSPSendMailToDSP",0x00000014,0xDE89B38C},
{"DSPAssertInt",0x00000040,0x3BA495D7},
{"DSPInit",0x000000C4,0xCEF041E4},
{"__DSP_debug_printf",0x00000050,0xB59B4703},
{"__DSP_exec_task",0x000001A0,0xB6364C5F},
{"__DSP_boot_task",0x0000018C,0x63CE5AA8},
{"__DSP_insert_task",0x000000A0,0xC8687795},
{"__DSP_remove_task",0x00000094,0x7626C2FE},
{"__CARDDefaultApiCallback",0x00000004,0x5032CF7F},
{"__CARDSyncCallback",0x00000034,0xA3B16A4A},
{"__CARDExtHandler",0x000000D8,0x122BBE43},
{"__CARDExiHandler",0x00000118,0x4E76986C},
{"__CARDTxHandler",0x000000A8,0x49EFF252},
{"__CARDUnlockedHandler",0x00000084,0xA2461B4C},
{"__CARDEnableInterrupt",0x000000C0,0x493A003A},
{"__CARDReadStatus",0x000000F0,0x3687D2A0},
{"__CARDClearStatus",0x000000AC,0xD8208EA1},
{"TimeoutHandler",0x000000A4,0xCBBB30E2},
{"Retry",0x0000022C,0xA2F9937E},
{"UnlockedCallback",0x00000110,0xE8BEAE69},
{"__CARDStart",0x000001B4,0x35AB85B3},
{"__CARDReadSegment",0x00000134,0x619B6785},
{"__CARDWritePage",0x0000011C,0x11D2B51F},
{"__CARDEraseSector",0x000000E0,0x5FAC8354},
{"CARDInit",0x000000AC,0x71897CBD},
{"__CARDSetDiskID",0x00000038,0x63EC8ED5},
{"__CARDGetControlBlock",0x000000B8,0xF19E004E},
{"__CARDPutControlBlock",0x00000064,0xF8BA004E},
{"CARDFreeBlocks",0x00000150,0xB4B71010},
{"__CARDSync",0x00000098,0xFB7DE91E},
{"OnReset",0x00000050,0x8F340DF5},
{"bitrev",0x0000016C,0xBD827A2E},
{"ReadArrayUnlock",0x00000144,0x27DE16B9},
{"DummyLen",0x000000C4,0x24E677DB},
{"__CARDUnlock",0x00000B58,0xBD7FDF55},
{"InitCallback",0x00000070,0x0B3B8832},
{"DoneCallback",0x00000324,0xF34F3D63},
{"BlockReadCallback",0x000000DC,0xC3012BDA},
{"__CARDRead",0x00000064,0x9D50F4C1},
{"BlockWriteCallback",0x000000DC,0xC3012BDA},
{"__CARDWrite",0x00000064,0x9D50F4C1},
{"__CARDGetFatBlock",0x00000008,0x85799EFF},
{"WriteCallback",0x000000D4,0x3437B3C2},
{"EraseCallback",0x000000C8,0x961620BC},
{"__CARDAllocBlock",0x00000118,0xC8702086},
{"__CARDUpdateFatBlock",0x000000AC,0x0644F934},
{"__CARDGetDirBlock",0x00000008,0x85799EFF},
{"WriteCallback",0x000000D0,0x966C0DF5},
{"EraseCallback",0x000000C8,0x961620BC},
{"__CARDUpdateDir",0x000000C4,0xA275937E},
{"__CARDCheckSum",0x000001B0,0xECA39344},
{"VerifyID",0x00000284,0xD2C70693},
{"VerifyDir",0x00000240,0xE090D433},
{"VerifyFAT",0x00000284,0x11161B38},
{"__CARDVerify",0x0000008C,0x6D4EC2FE},
{"CARDCheckExAsync",0x00000590,0x41DBC423},
{"CARDCheck",0x00000054,0x52EA5729},
{"CARDProbe",0x00000038,0x1999C419},
{"CARDProbeEx",0x0000017C,0x9BEBB3B6},
{"DoMount",0x00000410,0x4189BE79},
{"__CARDMountCallback",0x00000138,0x1F567A5A},
{"CARDMountAsync",0x000001A0,0x7DDDB3F8},
{"CARDMount",0x00000048,0xF8DD102A},
{"DoUnmount",0x0000009C,0x0AC7DF21},
{"CARDUnmount",0x000000AC,0xC4363BBE},
{"__CARDFormatRegionAsync",0x00000658,0x299B2B94},
{"CARDFormat",0x00000054,0x2D93B3C2},
{"__CARDCompareFileName",0x00000068,0xA2D16A4A},
{"__CARDAccess",0x00000098,0x69B79EB1},
{"__CARDIsPublic",0x00000030,0x5FF30B66},
{"CARDOpen",0x00000178,0x2C27E4D1},
{"CARDClose",0x00000054,0x4378D447},
{"CreateCallbackFat",0x00000130,0x12EB4777},
{"CARDCreateAsync",0x00000220,0x5A26986C},
{"CARDCreate",0x00000048,0xF8DD102A},
{"__CARDSeek",0x000001B8,0x09827A5A},
{"ReadCallback",0x00000130,0x12577106},
{"CARDReadAsync",0x00000148,0x5CE7C423},
{"CARDRead",0x00000048,0xF8DD102A},
{"WriteCallback",0x00000170,0x8C032BE0},
{"EraseCallback",0x000000B0,0xB1D24C5F},
{"CARDWriteAsync",0x00000114,0xE1BDBE0D},
{"CARDWrite",0x00000048,0xF8DD102A},
{"UpdateIconOffsets",0x000001F8,0x4EF9105E},
{"CARDGetStatus",0x0000012C,0x18DEE20C},
{"CARDSetStatusAsync",0x00000174,0xBAFF4703},
{"CARDSetStatus",0x00000048,0xF8DD102A},
{"CARDGetSerialNo",0x000000C4,0x76C20DBB},
{"__GXDefaultTexRegionCallback",0x0000007C,0x24E895D7},
{"__GXDefaultTlutRegionCallback",0x00000024,0x01078832},
{"GXInit",0x0000086C,0x78AA0DBB},
{"__GXInitGX",0x000008C4,0x24F85A92},
{"GXCPInterruptHandler",0x00000134,0xDBB26CAD},
{"GXInitFifoBase",0x0000006C,0x9CDA77AF},
{"GXInitFifoPtrs",0x00000070,0x2F68F4B5},
{"GXInitFifoLimits",0x0000000C,0x69CDB3F8},
{"GXSetCPUFifo",0x00000110,0x5E4C4C11},
{"GXSetGPFifo",0x00000178,0x766D4ACC},
{"GXSaveCPUFifo",0x00000020,0xB50AD409},
{"__GXSaveCPUFifoAux",0x000000DC,0x01996A3E},
{"GXGetGPStatus",0x00000050,0x4E9A6158},
{"__GXFifoInit",0x0000004C,0x6FA4EFB7},
{"__GXFifoReadEnable",0x00000024,0x96C1FF9D},
{"__GXFifoReadDisable",0x00000020,0x65463BCA},
{"__GXFifoLink",0x0000003C,0xC5CA5ADC},
{"__GXWriteFifoIntEnable",0x0000003C,0x07EBDF21},
{"__GXWriteFifoIntReset",0x0000003C,0xB44FDF55},
{"__GXCleanGPFifo",0x00000100,0x24B4A3A6},
{"GXSetCurrentGXThread",0x0000004C,0x6CCA3BCA},
{"GXGetCurrentGXThread",0x00000008,0xB802D409},
{"GXGetCPUFifo",0x00000008,0xB802D409},
{"GXGetGPFifo",0x00000008,0xB802D409},
{"__GXXfVtxSpecs",0x00000158,0x57CB5C75},
{"GXSetVtxDesc",0x00000350,0x3C4895D7},
{"__GXSetVCD",0x00000054,0x3FD59E8B},
{"GXClearVtxDesc",0x00000038,0x5004B525},
{"GXSetVtxAttrFmt",0x00000358,0x28CE41AA},
{"GXSetVtxAttrFmtv",0x00000378,0x191F4777},
{"__GXSetVAT",0x0000009C,0x14396A3E},
{"GXGetVtxAttrFmt",0x00000280,0x1BCED47D},
{"GXGetVtxAttrFmtv",0x00000074,0x19CAB51F},
{"GXSetArray",0x0000008C,0xD79B67F1},
{"GXInvalidateVtxCache",0x00000010,0xA85D0B28},
{"GXSetTexCoordGen2",0x000002D0,0x72B3A8C0},
{"GXSetNumTexGens",0x00000040,0x109A4C2B},
{"GXFlush",0x0000005C,0x35C2EF8D},
{"GXAbortFrame",0x000000B4,0x43239330},
{"GXSetDrawSync",0x000000B8,0x6950EFB7},
{"GXSetDrawDone",0x00000098,0x4A7AF90E},
{"GXDrawDone",0x00000080,0xD60FFFA7},
{"GXPixModeSync",0x00000024,0x64DB3096},
{"GXPokeAlphaMode",0x00000014,0x92C967CB},
{"GXPokeAlphaRead",0x00000014,0xA709937E},
{"GXPokeAlphaUpdate",0x0000001C,0xB741C46D},
{"GXPokeBlendMode",0x00000088,0xBE42E278},
{"GXPokeColorUpdate",0x0000001C,0xB741C46D},
{"GXPokeDstAlpha",0x00000014,0x69AAA39C},
{"GXPokeDither",0x0000001C,0xB741C46D},
{"GXPokeZMode",0x00000028,0x254A41AA},
{"GXPeekARGB",0x00000024,0xD46C8EA1},
{"GXPeekZ",0x00000028,0x84B46C97},
{"GXSetDrawSyncCallback",0x00000044,0xAF3CAE53},
{"GXTokenInterruptHandler",0x00000088,0x68DD8589},
{"GXSetDrawDoneCallback",0x00000044,0xAF3CAE53},
{"GXFinishInterruptHandler",0x00000084,0x8C20F4C1},
{"__GXPEInit",0x00000080,0xC0CBA8B4},
{"__GXSetDirtyState",0x000000A0,0x5AB7A541},
{"GXBegin",0x000000F0,0x6FA7C9A2},
{"__GXSendFlushPrim",0x00000088,0xA4E86116},
{"GXSetLineWidth",0x00000048,0x81345AE6},
{"GXSetPointSize",0x00000048,0xB9AA831A},
{"GXEnableTexOffsets",0x0000005C,0xA37A1B4C},
{"GXSetCullMode",0x0000004C,0x8E6E8E9B},
{"GXSetCoPlanar",0x0000003C,0xE3D57148},
{"__GXSetGenMode",0x00000024,0x64DB3096},
{"GXSetDispCopySrc",0x00000090,0x88D4C2B0},
{"GXSetTexCopySrc",0x00000090,0x88D4C2B0},
{"GXSetDispCopyDst",0x0000003C,0xB0D2B56B},
{"GXSetTexCopyDst",0x00000154,0xE40C2D07},
{"GXSetDispCopyFrame2Field",0x00000028,0x72D1E4EB},
{"GXSetCopyClamp",0x00000068,0xF0FD102A},
{"GXGetNumXfbLines",0x00000090,0x311530AC},
{"GXGetYScaleFactor",0x00000238,0x58988354},
{"GXSetDispCopyYScale",0x000000C8,0x33F72B94},
{"GXSetCopyClear",0x00000068,0xF256363F},
{"GXSetCopyFilter",0x00000228,0xB59C9822},
{"GXSetDispCopyGamma",0x0000001C,0x2D7F4AF6},
{"GXCopyDisp",0x0000015C,0x879C5AE6},
{"GXCopyTex",0x0000017C,0x13FFBE43},
{"GXClearBoundingBox",0x00000038,0xF9A1265B},
{"GXInitLightAttn",0x0000001C,0x48E5BE79},
{"GXInitLightSpot",0x00000190,0x8B072BE0},
{"GXInitLightDistAttn",0x000000D0,0x77E20DBB},
{"GXInitLightPos",0x00000010,0x77098589},
{"GXInitLightDir",0x0000001C,0x563E364B},
{"GXInitLightColor",0x0000000C,0xE99AF97A},
{"GXLoadLightObjImm",0x0000007C,0x77020DBB},
{"GXSetNumChans",0x00000044,0x07821B38},
{"GXSetChanCtrl",0x000000CC,0x5B4AF90E},
{"GXGetTexBufferSize",0x0000015C,0xCD92F4FB},
{"__GetImageTileCount",0x000000C8,0x03DA7A5A},
{"GXInitTexObj",0x00000274,0x12FB1064},
{"GXInitTexObjCI",0x00000048,0xE6017148},
{"GXInitTexObjLOD",0x00000194,0x9831C9EC},
{"GXGetTexObjData",0x0000000C,0xFF22612C},
{"GXGetTexObjWidth",0x00000010,0x90399EFF},
{"GXGetTexObjHeight",0x00000010,0x90399EFF},
{"GXGetTexObjFmt",0x00000008,0x85799EFF},
{"GXGetTexObjWrapS",0x0000000C,0xFF22612C},
{"GXGetTexObjWrapT",0x0000000C,0xFF22612C},
{"GXGetTexObjTlut",0x00000008,0x85799EFF},
{"GXLoadTexObjPreLoaded",0x00000198,0xF7B8B551},
{"GXInitTlutObj",0x00000048,0xA238CF31},
{"GXInitTexCacheRegion",0x00000120,0xB1D8CF31},
{"GXInitTlutRegion",0x00000048,0xDB791DAB},
{"GXInvalidateTexAll",0x00000048,0x88268E9B},
{"GXSetTexRegionCallback",0x00000014,0x692077E1},
{"GXSetTlutRegionCallback",0x00000014,0x692077E1},
{"GXSetTexCoordScaleManually",0x0000008C,0x0A1E4C2B},
{"GXSetTexCoordBias",0x0000007C,0x3DADA8FA},
{"__SetSURegs",0x000000B8,0x813B85C7},
{"__GXSetSUTexRegs",0x0000017C,0x2F659E8B},
{"__GXSetTmemConfig",0x00000240,0xEE107A14},
{"GXSetTevIndirect",0x0000009C,0xC9D5B38C},
{"GXSetIndTexMtx",0x00000160,0xB1DB4703},
{"GXSetIndTexCoordScale",0x0000017C,0xA1410B28},
{"GXSetIndTexOrder",0x00000114,0xC2FD7CC9},
{"GXSetNumIndStages",0x00000028,0x403A986C},
{"GXSetTevDirect",0x00000048,0x219F85B3},
{"GXSetTevIndWarp",0x00000064,0x1B4F8832},
{"__GXUpdateBPMask",0x000000CC,0xD2FC8EA1},
{"__GXFlushTextureState",0x00000024,0x64DB3096},
{"GXSetTevOp",0x0000008C,0xE017C457},
{"GXSetTevColorIn",0x00000044,0xD2D92BDA},
{"GXSetTevAlphaIn",0x00000044,0x674E95ED},
{"GXSetTevColorOp",0x00000068,0xDC3C2086},
{"GXSetTevAlphaOp",0x00000068,0xDC3C2086},
{"GXSetTevColor",0x00000074,0x3B84F4B5},
{"GXSetTevColorS10",0x00000074,0x7B0F9EB1},
{"GXSetTevKColor",0x00000074,0x41B2003A},
{"GXSetTevKColorSel",0x0000006C,0xD3DF0693},
{"GXSetTevKAlphaSel",0x0000006C,0x823C95A3},
{"GXSetTevSwapMode",0x00000054,0xBE3C3605},
{"GXSetTevSwapModeTable",0x00000098,0xC35A5ADC},
{"GXSetAlphaCompare",0x00000054,0xE787A535},
{"GXSetZTexture",0x00000084,0x66457CBD},
{"GXSetTevOrder",0x000001D8,0x10BCCF45},
{"GXSetNumTevStages",0x00000030,0x30274AF6},
{"GXSetFog",0x000001B0,0x0CEE1B38},
{"GXSetFogRangeAdj",0x00000100,0xC86BFFA7},
{"GXSetBlendMode",0x00000054,0xD3E7A8B4},
{"GXSetColorUpdate",0x0000002C,0x212DC998},
{"GXSetAlphaUpdate",0x0000002C,0x16362D49},
{"GXSetZMode",0x00000034,0x106D6A3E},
{"GXSetZCompLoc",0x00000038,0x689D2BAE},
{"GXSetPixelFmt",0x000000E8,0x14CC6162},
{"GXSetDither",0x00000034,0xB19F1010},
{"GXSetDstAlpha",0x0000003C,0x9C9530D8},
{"GXSetFieldMask",0x00000038,0x4AD4B525},
{"GXSetFieldMode",0x0000007C,0x84B216CD},
{"__GXSetRange",0x00000004,0x5032CF7F},
{"GXCallDisplayList",0x00000070,0x77401683},
{"GXProject",0x00000174,0x5EE2986C},
{"GXSetProjection",0x000000B4,0xB6FA4C5F},
{"GXSetProjectionv",0x000000C0,0x1251A57B},
{"GXGetProjectionv",0x00000060,0x9F3DC9EC},
{"GXLoadPosMtxImm",0x00000050,0xFFFB5C01},
{"GXLoadNrmMtxImm",0x00000050,0x0119A57B},
{"GXSetCurrentMtx",0x00000038,0x75E98589},
{"GXLoadTexMtxImm",0x000000B4,0x28C74AF6},
{"GXSetViewportJitter",0x00000104,0xEA1F9344},
{"GXSetViewport",0x00000024,0xD1888EA1},
{"GXGetViewportv",0x00000038,0x1B87DF21},
{"GXSetScissor",0x00000090,0x1EEF4777},
{"GXGetScissor",0x00000048,0x3C6241AA},
{"GXSetScissorBoxOffset",0x00000040,0xA6785767},
{"GXSetClipMode",0x00000028,0x39EAD9FC},
{"__GXSetMatrixIndex",0x00000084,0x4E1C4C11},
{"GXSetGPMetric",0x00000848,0x66412BAE},
{"GXClearGPMetric",0x00000010,0x6B59E4EB},
{"GXReadXfRasMetric",0x000000C4,0xB49CCF31},
{"GDInitGDLObj",0x00000018,0x944095A3},
{"GDFlushCurrToMem",0x0000002C,0x01024C2B},
{"GDPadCurr32",0x000000F8,0x099B1064},
{"GDOverflowed",0x00000030,0x15A9A57B},
{"GDSetVtxDescv",0x000005FC,0xDF51D2EE},
{"GDSetArray",0x0000020C,0xF6FCB551},
{"GDSetArrayRaw",0x00000208,0x02B1F268},
{"__copy",0x00000030,0xC0139EC5},
{"exit",0x000000CC,0x019C9856},
{"free",0x00000134,0x9116EFF9},
{"div",0x00000058,0x1E922D49},
{"abs",0x00000010,0x4FB0D447},
{"printf",0x000000CC,0xBB5F1010},
{"cos",0x000000D4,0x6C1EC2FE},
{"floor",0x00000148,0x59D1474D},
{"tan",0x00000078,0x04562D49},
{"EXI2_Init",0x00000004,0x5032CF7F},
{"EXI2_EnableInterrupts",0x00000004,0x5032CF7F},
{"EXI2_Poll",0x00000008,0x0D03BE43},
{"EXI2_ReadN",0x00000008,0x0D03BE43},
{"EXI2_WriteN",0x00000008,0x0D03BE43},
{"EXI2_Reserve",0x00000004,0x5032CF7F},
{"EXI2_Unreserve",0x00000004,0x5032CF7F},
{"AMC_IsStub",0x00000008,0x0D03BE43},
{"DBClose",0x00000004,0x5032CF7F},
{"DBOpen",0x00000004,0x5032CF7F},
{"DBWrite",0x00000260,0x03B1930A},
{"DBRead",0x0000008C,0x7DBF51F4},
{"DBQueryData",0x0000009C,0x14867A5A},
{"DBInitInterrupts",0x00000054,0xE35C8320},
{"DBInitComm",0x00000078,0x6D77FFD3},
{"Hu_IsStub",0x00000008,0x0D03BE43},
{NULL,0,0}
};

struct HLEPatchData
{
	const char *	FuncName;
	u32		FuncSize;
	u32		FuncHash;
	char 	PatchFuncName[128];
};

HLEPatchData HLE_CRCPatch[] =
{
{"__OSInitAudioSystem",0x00000194,0x4D2D262F,"ignore"},
{"__OSInitAudioSystem",0x000001BC,0xDFC2C28A,"ignore"},
{"__OSStopAudioSystem",0x000000D8,0x262C95D7,"ignore"},
{"__AXOutInitDSP",0x000000C4,0x286385B3,"ignore"},
{"__AXOutInitDSP",0x000000CC,0x245B4AF6,"ignore"},
{"salInitDSP",0x000000C0,0x67960DBB,"ignore_return_true"},
{"salInitDSP",0x000000BC,0x7ECFA8C0,"ignore_return_true"},
{"OSReport",0x00000080,0x1A446162,""},
{"OSReport",0x00000080,0x4F6A6158,""},
{"OSPanic",0x00000138,0xAD2C6116,""},
{"DBPrintf",0x00000050,0xB59B4703,""},
//{"DVDOpen",0x000000C8,0x239BD2A0,""},
{NULL,0,0}
};
