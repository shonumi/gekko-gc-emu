#include "emu.h"

extern u32 crc32_table[4][256];

void Init_CRC32_Table();
u32 GenerateCRC(u8 *StartAddr, u32 Len);
