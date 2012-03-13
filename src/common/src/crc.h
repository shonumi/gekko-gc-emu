#ifndef __CRC_H_
#define __CRC_H_

extern u32 crc32_table[4][256];

void Init_CRC32_Table();
u32 GenerateCRC(u8 *StartAddr, u32 Len);

#endif // __CRC_H_