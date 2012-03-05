////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_assembler_jumps.cpp
// DESC:		Assembler of the IL jump opcodes for the recompiler
// CREATED:		Mar 3, 2008
////////////////////////////////////////////////////////////
// Copyright (c) 2008 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"

u32		GekkoCPURecompiler::NextJumpID_Val;

GekkoRecILOp(JUMPLABEL)
{
	JumpList *NewEntry;

	//add a label to our label list for the current location in memory
	NewEntry = (JumpList *)RecompileAlloc(sizeof(JumpList));
	NewEntry->Next = JumpLabelEntries;
	NewEntry->ID = Instruction->InVal;
	NewEntry->Address = (u32)OutInstruction;

	//add it to the beginning of the tree
	JumpLabelEntries = NewEntry;

	//no data added
	*OutSize = 0;
}

GekkoRecILOp(JUMP)
{
	JumpList	*LabelEntry;
	JumpList	JumpData;

	//find the label entry for our ID
	LabelEntry = JumpLabelEntries;
	while(LabelEntry)
	{
		if((LabelEntry->ID & 0x7FFFFFFF) == Instruction->InVal)
			break;

		LabelEntry = LabelEntry->Next;
	}

	//create the jump data
	JumpData.Address = (u32)OutInstruction;
	JumpData.ID = Instruction->InVal;

	//if not JMP then 6 bytes, otherwise 5 bytes
	if(Instruction->OutReg != 0xE9)
	{
		//add the bytes for the jump command
		((u16 *)OutInstruction)[0] = (u16)Instruction->OutReg;
		*OutSize = 6;
	}
	else
	{
		//add the byte for the jump command
		((u8 *)OutInstruction)[0] = (u8)Instruction->OutReg;
		*OutSize = 5;
		JumpData.ID |= 0x80000000;		//flag JMP
	}

	//process the jump if we found the label
	if(LabelEntry)
		ProcessJump(LabelEntry, &JumpData);
	else
	{
		//did not find the label, add our jump data to the jump tree
		LabelEntry = (JumpList *)RecompileAlloc(sizeof(JumpList));
		memcpy(LabelEntry, &JumpData, sizeof(JumpList));
		LabelEntry->Next = JumpEntries;

		//add to the beginning of the tree
		JumpEntries = LabelEntry;
	}
}

void GekkoCPURecompiler::ProcessJump(JumpList *LabelEntry, JumpList *JumpEntry)
{
	u8	*OutData;
	s32	JumpDistance;

	//pointer to the area to write to
	OutData = (u8 *)JumpEntry->Address + 2;


	//calculate jump and verify it is valid before assigning
	JumpDistance = LabelEntry->Address - JumpEntry->Address - 6;

	//if JMP then increment 1 byte
	if(JumpEntry->ID & 0x80000000)
	{
		(OutData)--;
		JumpDistance++;
	}

/*	if((JumpDistance < -128) || (JumpDistance > 128))
	{
		printf("Jump distance %d for label %d, address 0x%08X from jump 0x%08X is invalid!\n", JumpDistance, LabelEntry->ID, LabelEntry->Address, JumpEntry->Address);
		cpu->pause = true;
	}
	else
*/		*(s32 *)OutData = (s32)JumpDistance;
}