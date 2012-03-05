#define ModRM(Mod, OutReg, InReg)	(((Mod)<<6) | ((OutReg) << 3) | (InReg))

#define Instruction1Param(InstrName, InstrVal, ModVal)								\
GekkoRecILOp(##InstrName)															\
{																					\
	switch(Instruction->Flags & RecInstrFlagX86_INMASK)								\
	{																				\
		case (RecInstrFlagX86InReg):												\
			*(u16 *)OutInstruction = (ModRM(0x03, ModVal,							\
									 (Instruction->X86InReg & ~REG_X86_CACHE)) << 8)\
									 | InstrVal;									\
		    X86Regs[(Instruction->X86InReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
			*OutSize = 2;															\
			break;																	\
																					\
		case (RecInstrFlagX86InMem):												\
			*(u16 *)OutInstruction = (ModRM(0x00, ModVal, 0x05) << 8) | InstrVal;	\
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InMemory;		\
			*OutSize = 6;															\
			break;																	\
																					\
		default:																	\
		Unknown_Mask(#InstrName, (Instruction->Flags & RecInstrFlagX86_MASK));		\
			break;																	\
	}																				\
}

#define Instruction2Param(InstrName, InstrVal_MemReg, InstrVal_RegMem, InstrVal_InVal, InstrVal_InValMod)	\
GekkoRecILOp(##InstrName)																		\
{																								\
	switch(Instruction->Flags & RecInstrFlagX86_MASK)											\
	{																							\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):									\
			*(u16 *)OutInstruction = (ModRM(0x03, (Instruction->X86InReg & ~REG_X86_CACHE),		\
									 (Instruction->X86OutReg & ~REG_X86_CACHE)) <<	8)			\
									 | InstrVal_MemReg;											\
		    X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;				\
			*OutSize = 2;																		\
			break;																				\
																								\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86OutMem):			\
			if(Instruction->X86Displacement)													\
			{																					\
				if((Instruction->X86Displacement < -128) || (Instruction->X86Displacement > 127))	\
				{																					\
					*(u16 *)OutInstruction = (ModRM(0x01, (Instruction->X86InReg & ~REG_X86_CACHE),	\
											 (Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)	\
											| InstrVal_MemReg;									\
					*(u32 *)(&((u8 *)OutInstruction)[2]) = (u32)Instruction->X86Displacement;	\
					X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
					*OutSize = 6;																\
				}																				\
				else																			\
				{																					\
					*(u16 *)OutInstruction = (ModRM(0x01, (Instruction->X86InReg & ~REG_X86_CACHE),	\
											 (Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)		\
											| InstrVal_MemReg;									\
					*(u8 *)(&((u8 *)OutInstruction)[2]) = (u8)Instruction->X86Displacement;		\
					X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
					*OutSize = 3;																\
				}																				\
			}																					\
			else																				\
			{																					\
				*(u16 *)OutInstruction = (ModRM(0x00, (Instruction->X86InReg & ~REG_X86_CACHE),	\
										 (Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)		\
										| InstrVal_MemReg;										\
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;								\
				*OutSize = 2;																	\
			}																					\
			break;																				\
																								\
		case (RecInstrFlagX86InReg | RecInstrFlagX86InMem | RecInstrFlagX86OutReg):				\
			if(Instruction->X86Displacement)													\
			{																					\
				if((Instruction->X86Displacement < -128) || (Instruction->X86Displacement > 127))	\
				{																					\
					*(u16 *)OutInstruction = (ModRM(0x01, (Instruction->X86InReg & ~REG_X86_CACHE),	\
											 (Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)		\
											| InstrVal_RegMem;									\
					*(u32 *)(&((u8 *)OutInstruction)[2]) = (u32)Instruction->X86Displacement;	\
					X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
					*OutSize = 6;																\
				}																				\
				else																			\
				{																				\
					*(u16 *)OutInstruction = (ModRM(0x01, (Instruction->X86InReg & ~REG_X86_CACHE),	\
											 (Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)		\
											| InstrVal_RegMem;										\
					*(u8 *)(&((u8 *)OutInstruction)[2]) = (u8)Instruction->X86Displacement;		\
					X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
					*OutSize = 3;																\
				}																				\
			}																					\
			else																				\
			{																					\
				*(u16 *)OutInstruction = (ModRM(0x00, (Instruction->X86InReg & ~REG_X86_CACHE),	\
										 (Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)		\
										| InstrVal_RegMem;									\
				X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
				*OutSize = 2;																\
			}																				\
			break;																		\
																						\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):							\
			if(Instruction->X86Displacement)													\
			{																					\
				if((Instruction->X86Displacement < -128) || (Instruction->X86Displacement > 127))	\
				{																					\
					*(u16 *)OutInstruction = (ModRM(0x02,											\
											(Instruction->X86InReg & ~REG_X86_CACHE), 0x05) << 8)	\
											| InstrVal_MemReg;									\
					*(u32 *)(&((u8 *)OutInstruction)[2]) = (u32)Instruction->X86Displacement;	\
					*(u32 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86OutMemory;			\
					*OutSize = 10;																\
				}																				\
				else																			\
				{																				\
					*(u16 *)OutInstruction = (ModRM(0x01,											\
											(Instruction->X86InReg & ~REG_X86_CACHE), 0x05) << 8)	\
											| InstrVal_MemReg;										\
					*(u8 *)(&((u8 *)OutInstruction)[2]) = (u8)Instruction->X86Displacement;		\
					*(u32 *)(&((u8 *)OutInstruction)[3]) = Instruction->X86OutMemory;			\
					*OutSize = 7;																\
				}																				\
			}																					\
			else																				\
			{																					\
				*(u16 *)OutInstruction = (ModRM(0x00,											\
										(Instruction->X86InReg & ~REG_X86_CACHE), 0x05) << 8)	\
										| InstrVal_MemReg;									\
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;			\
				*OutSize = 6;																\
			}																				\
			break;																			\
																							\
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):								\
			*(u16 *)OutInstruction = (ModRM(0x00,											\
									(Instruction->X86OutReg & ~REG_X86_CACHE), 0x05) << 8)	\
									 | InstrVal_RegMem;										\
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InMemory;				\
		    X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;			\
			*OutSize = 6;																\
			break;																		\
																						\
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):							\
			*(u16 *)OutInstruction = (ModRM(0x03, InstrVal_InValMod,					\
									(Instruction->X86OutReg & ~REG_X86_CACHE)) << 8)	\
									 | InstrVal_InVal;									\
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal;				\
			X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
			*OutSize = 6;																\
			break;																		\
																						\
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):							\
			*(u16 *)OutInstruction = (ModRM(0x00, InstrVal_InValMod, 0x05) << 8)		\
									 | InstrVal_InVal;									\
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;			\
			*(u32 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal;				\
			*OutSize = 10;																\
			break;																		\
																						\
		default:																		\
		Unknown_Mask(#InstrName, (Instruction->Flags & RecInstrFlagX86_MASK));			\
			break;																		\
	}																					\
}

#define Instruction2Byte(InstrName, InstrVal_MemReg, InstrVal_RegMem, InstrVal_InVal, InstrVal_InValMod)	\
GekkoRecILOp(##InstrName)																\
{																						\
	switch(Instruction->Flags & RecInstrFlagX86_MASK)									\
	{																					\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):							\
			*(u32 *)OutInstruction = (ModRM(0x03, (Instruction->X86InReg & ~REG_X86_CACHE),	\
									(Instruction->X86OutReg & ~REG_X86_CACHE)) << 16)		\
									 | InstrVal_MemReg;									\
		    X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
			*OutSize = 3;																\
			break;																		\
																						\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):							\
			*(u32 *)OutInstruction = (ModRM(0x00,											\
									(Instruction->X86InReg & ~REG_X86_CACHE), 0x05) << 16)	\
									 | InstrVal_MemReg;										\
			*(u32 *)(&((u8 *)OutInstruction)[3]) = Instruction->X86OutMemory;			\
			*OutSize = 7;																\
			break;																		\
																						\
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):							\
			*(u32 *)OutInstruction = (ModRM(0x00,											\
									(Instruction->X86OutReg & ~REG_X86_CACHE), 0x05) << 16)	\
									 | InstrVal_RegMem;										\
			*(u32 *)(&((u8 *)OutInstruction)[3]) = Instruction->X86InMemory;			\
			X86Regs[(Instruction->X86OutReg & ~REG_X86_CACHE)].ValueChanged = 1;		\
			*OutSize = 7;																\
			break;																		\
																						\
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):							\
			*(u32 *)OutInstruction = (ModRM(0x03, InstrVal_InValMod,					\
									  (Instruction->X86OutReg & ~REG_X86_CACHE)) << 16)	\
									 | InstrVal_InVal;									\
			*(u32 *)(&((u8 *)OutInstruction)[3]) = Instruction->X86InVal;				\
			*OutSize = 7;																\
			break;																		\
																						\
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):							\
			*(u32 *)OutInstruction = (ModRM(0x00, InstrVal_InValMod, 0x05) << 16)		\
									 | InstrVal_InVal;									\
			*(u32 *)(&((u8 *)OutInstruction)[3]) = Instruction->X86OutMemory;			\
			*(u32 *)(&((u8 *)OutInstruction)[7]) = Instruction->X86InVal;				\
			*OutSize = 11;																\
			break;																		\
																						\
		default:																		\
		Unknown_Mask(#InstrName, (Instruction->Flags & RecInstrFlagX86_MASK));			\
			break;																		\
	}																					\
}

#define FPUInstruction2Byte(InstrName, InstrCmd)						\
GekkoRecILOp(##InstrName)												\
{																		\
	*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86InReg,		\
									Instruction->X86OutReg) << 16)		\
								| InstrCmd;								\
	FPURegs[Instruction->X86OutReg].ValueChanged = 1;					\
	*OutSize = 3;														\
}

#define FPUInstruction2ByteInverse(InstrName, InstrCmd)					\
GekkoRecILOp(##InstrName)												\
{																		\
	*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86OutReg,		\
									Instruction->X86InReg) << 16)		\
								| InstrCmd;								\
	FPURegs[Instruction->X86OutReg].ValueChanged = 1;					\
	*OutSize = 3;														\
}

#define FPUInstruction3Byte(InstrName, InstrCmd)						\
GekkoRecILOp(##InstrName)												\
{																		\
	*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86OutReg,		\
									Instruction->X86InReg) << 24)		\
								| InstrCmd;								\
	FPURegs[Instruction->X86OutReg].ValueChanged = 1;					\
	*OutSize = 4;														\
}

#define FPUInstruction3ByteImm8(InstrName, InstrCmd)							\
GekkoRecILOp(##InstrName)														\
{																				\
	*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86OutReg,				\
									Instruction->X86InReg) << 24)				\
								| InstrCmd;										\
	*(u8 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86CmdImmediateVal;		\
	FPURegs[Instruction->X86OutReg].ValueChanged = 1;							\
	*OutSize = 5;																\
}

#define FPUInstructionMem3Byte(InstrName, InstrCmdReg, InstrCmdRegMem)									\
GekkoRecILOp(##InstrName)																				\
{																										\
	switch(Instruction->Flags & RecInstrFlagX86_MASK)													\
	{																									\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):											\
			*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86OutReg,								\
									Instruction->X86InReg) << 24) | InstrCmdReg;						\
			*OutSize = 4;																				\
																										\
			if((Instruction->Flags & RecInstrFlagPPC_MASK) ==											\
				(RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg))								\
			{																							\
				if(Instruction->InReg != Instruction->OutReg)											\
					FPURegs[Instruction->X86OutReg].ValueChanged = 1;									\
			}																							\
			else																						\
			{																							\
				if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutFloatReg)		\
					FPURegs[Instruction->X86OutReg].ValueChanged = 1;									\
			}																							\
			break;																						\
																										\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):											\
			*(u32 *)OutInstruction = (ModRM(0x00, Instruction->X86InReg,								\
									0x05) << 24) | InstrCmdRegMem;										\
			*(u32 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86OutMemory;							\
			*OutSize = 8;																				\
			break;																						\
																										\
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):											\
			*(u32 *)OutInstruction = (ModRM(0x00, Instruction->X86OutReg,								\
									0x05) << 24) | InstrCmdReg;											\
			*(u32 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86InMemory;							\
			if((Instruction->Flags & RecInstrFlagPPC_MASK) ==											\
				(RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg))								\
			{																							\
				if(Instruction->InReg != Instruction->OutReg)											\
					FPURegs[Instruction->X86OutReg].ValueChanged = 1;									\
			}																							\
			else																						\
			{																							\
				if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutFloatReg)		\
					FPURegs[Instruction->X86OutReg].ValueChanged = 1;									\
			}																							\
			*OutSize = 8;																				\
			break;																						\
																										\
		default:																						\
			Unknown_Mask(#InstrName, (Instruction->Flags & RecInstrFlagX86_MASK));						\
			break;																						\
	}																									\
}

#define FPUInstructionMove3Byte(InstrName, InstrCmdReg, InstrCmdRegMem)									\
GekkoRecILOp(##InstrName)																				\
{																										\
	switch(Instruction->Flags & RecInstrFlagX86_MASK)													\
	{																									\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):											\
			if(Instruction->X86InReg == Instruction->X86OutReg)											\
				*OutSize = 0;																			\
			else																						\
			{																							\
				*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86OutReg,							\
										Instruction->X86InReg) << 24) | InstrCmdReg;					\
				*OutSize = 4;																			\
																										\
				if((Instruction->Flags & RecInstrFlagPPC_MASK) ==										\
					(RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg))							\
				{																						\
					if(Instruction->InReg != Instruction->OutReg)										\
						FPURegs[Instruction->X86OutReg].ValueChanged = 1;								\
				}																						\
				else																					\
				{																						\
					if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutFloatReg)	\
						FPURegs[Instruction->X86OutReg].ValueChanged = 1;								\
				}																						\
			}																							\
			break;																						\
																										\
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):											\
			*(u32 *)OutInstruction = (ModRM(0x00, Instruction->X86InReg,								\
									0x05) << 24) | InstrCmdRegMem;										\
			*(u32 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86OutMemory;							\
			*OutSize = 8;																				\
			break;																						\
																										\
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):											\
			*(u32 *)OutInstruction = (ModRM(0x00, Instruction->X86OutReg,								\
									0x05) << 24) | InstrCmdReg;											\
			*(u32 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86InMemory;							\
			if((Instruction->Flags & RecInstrFlagPPC_MASK) ==											\
				(RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg))								\
			{																							\
				if(Instruction->InReg != Instruction->OutReg)											\
					FPURegs[Instruction->X86OutReg].ValueChanged = 1;									\
			}																							\
			else																						\
			{																							\
				if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutFloatReg)		\
					FPURegs[Instruction->X86OutReg].ValueChanged = 1;									\
			}																							\
			*OutSize = 8;																				\
			break;																						\
																										\
		default:																						\
			Unknown_Mask(#InstrName, (Instruction->Flags & RecInstrFlagX86_MASK));						\
			break;																						\
	}																									\
}