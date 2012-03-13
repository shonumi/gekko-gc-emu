GekkoCPURecompiler::GekkoCPURecOpsGroup(Groups)[] =
{
	{GekkoCPURecompiler::GekkoCPUOpset, {4, GekkoRec(Ops_Group4)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {0, GekkoRec(Ops_Group4XO0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {8, GekkoRec(Ops_Group4XO0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {16, GekkoRec(Ops_Group4XO0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {22, GekkoRec(Ops_Group4XO0)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {19, GekkoRec(Ops_Group19)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {31, GekkoRec(Ops_Group31)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {59, GekkoRec(Ops_Group59)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {63, GekkoRec(Ops_Group63)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {6, GekkoRec(Ops_Group63XO0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {7, GekkoRec(Ops_Group63XO0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {8, GekkoRec(Ops_Group63XO0)}},

	{0, {0, 0}}
};

GekkoCPURecompiler::GekkoCPURecOpsGroup(Math)[] =
{
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {266, GekkoRec(ADD)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {266 | OE_T, GekkoRec(ADDO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {10, GekkoRec(ADDC)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {10 | OE_T, GekkoRec(ADDCO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {138, GekkoRec(ADDE)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {138 | OE_T, GekkoRec(ADDEO)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {14, GekkoRec(ADDI)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {12, GekkoRec(ADDIC)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {13, GekkoRec(ADDICD)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {15, GekkoRec(ADDIS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {234, GekkoRec(ADDME)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {234 | OE_T, GekkoRec(ADDMEO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {202, GekkoRec(ADDZE)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {202 | OE_T, GekkoRec(ADDZEO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {28, GekkoRec(AND)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {60, GekkoRec(ANDC)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {28, GekkoRec(ANDID)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {29, GekkoRec(ANDISD)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {491, GekkoRec(DIVW)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {459, GekkoRec(DIVWU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {954, GekkoRec(EXTSB)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {922, GekkoRec(EXTSH)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {284, GekkoRec(EQV)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {7, GekkoRec(MULLI)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {235, GekkoRec(MULLW)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {476, GekkoRec(NAND)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {104, GekkoRec(NEG)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {104 | OE_T, GekkoRec(NEGO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {124, GekkoRec(NOR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {444, GekkoRec(OR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {412, GekkoRec(ORC)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {24, GekkoRec(ORI)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {25, GekkoRec(ORIS)}},
//	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {24, GekkoRec(SLW)}},
//	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {824, GekkoRec(SRAWI)}},
//	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {536, GekkoRec(SRW)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {40, GekkoRec(SUBF)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {8, GekkoRec(SUBFC)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {8 | OE_T, GekkoRec(SUBFCO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {136, GekkoRec(SUBFE)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {136 | OE_T, GekkoRec(SUBFEO)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {8, GekkoRec(SUBFIC)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {232, GekkoRec(SUBFME)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {232 | OE_T, GekkoRec(SUBFMEO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {40 | OE_T, GekkoRec(SUBFO)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {200, GekkoRec(SUBFZE)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {316, GekkoRec(XOR)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {26, GekkoRec(XORI)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {27, GekkoRec(XORIS)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {20, GekkoRec(RLWIMI)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {21, GekkoRec(RLWINM)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {23, GekkoRec(RLWNM)}},
	{0, {0, 0}}
}; 

GekkoCPURecompiler::GekkoCPURecOpsGroup(LoadStore)[] =
{

	{GekkoCPURecompiler::GekkoCPUOpset, {34, GekkoRec(LBZ)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {35, GekkoRec(LBZU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {119, GekkoRec(LBZUX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {87, GekkoRec(LBZX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {42, GekkoRec(LHA)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {343, GekkoRec(LHAX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {40, GekkoRec(LHZ)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {41, GekkoRec(LHZU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {311, GekkoRec(LHZUX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {279, GekkoRec(LHZX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {46, GekkoRec(LMW)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {32, GekkoRec(LWZ)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {33, GekkoRec(LWZU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {35, GekkoRec(LWZUX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {23, GekkoRec(LWZX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {83, GekkoRec(MFMSR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {339, GekkoRec(MFSPR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {595, GekkoRec(MFSR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {371, GekkoRec(MFTB)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {146, GekkoRec(MTMSR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {467, GekkoRec(MTSPR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {210, GekkoRec(MTSR)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {38, GekkoRec(STB)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {39, GekkoRec(STBU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {215, GekkoRec(STBX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {247, GekkoRec(STBUX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {44, GekkoRec(STH)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {45, GekkoRec(STHU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {407, GekkoRec(STHX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {439, GekkoRec(STHUX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {47, GekkoRec(STMW)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {36, GekkoRec(STW)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {37, GekkoRec(STWU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {151, GekkoRec(STWX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {183, GekkoRec(STWUX)}},
	{0, {0, 0}}
}; 


GekkoCPURecompiler::GekkoCPURecOpsGroup(Branch)[] =
{
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {16, GekkoRec(BCLRX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {528, GekkoRec(BCCTRX)}},
	{GekkoCPURecompiler::GekkoCPUOpset,	{16, GekkoRec(BCX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {18, GekkoRec(BX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {470, GekkoRec(DCBI)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {50, GekkoRec(RFI)}},
	{0, {0, 0}}
}; 

GekkoCPURecompiler::GekkoCPURecOpsGroup(Compare)[] =
{
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {0, GekkoRec(CMP)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {32, GekkoRec(CMPL)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {10, GekkoRec(CMPLI)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {11, GekkoRec(CMPI)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {26, GekkoRec(CNTLZW)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {257, GekkoRec(CRAND)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {129, GekkoRec(CRANDC)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {289, GekkoRec(CREQV)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {225, GekkoRec(CRNAND)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {33, GekkoRec(CRNOR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {449, GekkoRec(CROR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {417, GekkoRec(CRORC)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {193, GekkoRec(CRXOR)}},
	//dcbi changes the function pointer, needs to be a branch but needs to be handled properly if it does/doesn't
	//change the PC
	//{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {470, GekkoRec(DCBI)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {19, GekkoRec(MFCR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {144, GekkoRec(MTCRF)}},
	{0, {0, 0}}
}; 

GekkoCPURecompiler::GekkoCPURecOpsGroup(NOP)[] =
{
	{GekkoCPURecompiler::GekkoCPUOpsGroup19Table, {150, GekkoRec(NOP)}},	//isync
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {982, GekkoRec(NOP)}},	//icbi
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {86, GekkoRec(NOP)}},		//dcbf
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {54, GekkoRec(NOP)}},		//dcbst
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {278, GekkoRec(NOP)}},	//dcbt
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {246, GekkoRec(NOP)}},	//dcbtst
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {1014, GekkoRec(NOP)}},	//dcbz
	{GekkoCPURecompiler::GekkoCPUOpsGroup4XO0Table, {1014, GekkoRec(NOP)}},	//dcbz_l
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {598, GekkoRec(NOP)}},	//sync
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {306, GekkoRec(NOP)}},	//tlbie
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {370, GekkoRec(NOP)}},	//tlbia
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {566, GekkoRec(NOP)}},	//tlbsync
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {4, GekkoRec(NOP)}},		//tw
	{GekkoCPURecompiler::GekkoCPUOpset, {2, GekkoRec(NOP)}},				//twi
	{0, {0, 0}}
}; 

GekkoCPURecompiler::GekkoCPURecOpsGroup(FPULoadStore)[] = 
{
	{GekkoCPURecompiler::GekkoCPUOpset, {50, GekkoRec(LFD)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {48, GekkoRec(LFS)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {49, GekkoRec(LFSU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {535, GekkoRec(LFSX)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {54, GekkoRec(STFD)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {52, GekkoRec(STFS)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {53, GekkoRec(STFSU)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {695, GekkoRec(STFSUX)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup31Table, {663, GekkoRec(STFSX)}},
	{0, {0, 0}}
};

GekkoCPURecompiler::GekkoCPURecOpsGroup(FPUMath)[] = 
{
	{GekkoCPURecompiler::GekkoCPUOpsGroup63XO0Table, {264, GekkoRec(FABS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {21, GekkoRec(FADD)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup59Table, {21, GekkoRec(FADDS)}},
//	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {18, GekkoRec(FDIV)}},
//	{GekkoCPURecompiler::GekkoCPUOpsGroup59Table, {18, GekkoRec(FDIVS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {29, GekkoRec(FMADD)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup59Table, {29, GekkoRec(FMADDS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63XO0Table, {72, GekkoRec(FMR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {28, GekkoRec(FMSUB)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup59Table, {28, GekkoRec(FMSUBS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {25, GekkoRec(FMUL)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup59Table, {25, GekkoRec(FMULS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63XO0Table, {40, GekkoRec(FNEG)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {12, GekkoRec(FRSP)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup63Table, {20, GekkoRec(FSUB)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup59Table, {20, GekkoRec(FSUBS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4XO0Table, {264, GekkoRec(PS_ABS)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {21, GekkoRec(PS_ADD)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {18, GekkoRec(PS_DIV)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {29, GekkoRec(PS_MADD)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {14, GekkoRec(PS_MADDS0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {15, GekkoRec(PS_MADDS1)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4XO0Table, {528, GekkoRec(PS_MERGE00)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4XO0Table, {560, GekkoRec(PS_MERGE01)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4XO0Table, {72, GekkoRec(PS_MR)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {28, GekkoRec(PS_MSUB)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {25, GekkoRec(PS_MUL)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {12, GekkoRec(PS_MULS0)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4XO0Table, {40, GekkoRec(PS_NEG)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {20, GekkoRec(PS_SUB)}},
	{GekkoCPURecompiler::GekkoCPUOpsGroup4Table, {10, GekkoRec(PS_SUM0)}},
	{GekkoCPURecompiler::GekkoCPUOpset, {56, GekkoRec(PSQ_L)}},
	{0, {0, 0}}
};