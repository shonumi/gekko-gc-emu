#ifndef VIDEO_CORE_TEXTURE_DECODER_H_
#define VIDEO_CORE_TEXTURE_DECODER_H_

#define TMEM_SIZE						0x100000
#define TMEM_MASK						0x0fffff
#define TEX_CACHE_LOCATION(n)       ((n & 0xffff) ^ (n >> 16))
#define PTEXTURE_MEM(addr)          &gp::tmem[addr & TMEM_MASK]
#define GX_TX_SETIMAGE_NUM(x)		(((x & 0x20) >> 3) | (x & 3))

namespace gp {

extern u8 tmem[TMEM_SIZE];
extern u32 texcache[TMEM_SIZE];

void DecodeTexture(u8 format, u32 addr, u16 height, u16 width);

} // namespace

#endif // VIDEO_CORE_TEXTURE_DECODER_H_