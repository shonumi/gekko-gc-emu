// ppc_disasm.h

#ifndef _PPC_DISASM_H_
#define _PPC_DISASM_H_

//

#define BRANCH_OPCODE       1
#define LDST_OPCODE         2

//


int EMU_FASTCALL DisassembleGekko(
    char *opcodeStr, 
    char *operandStr, 
    u32 opcode, 
    u32 curInstAddr, 
    u32 *nextInstAddr);

//

#endif
