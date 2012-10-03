// gx_texture.cpp
// (c) 2005,2009 Gekko Team

#include "sys/stat.h"
#include "common.h"
#include "config.h"
#include "memory.h"
#include "hw/hw_gx.h"
#include "bp_mem.h"
#include "texture_decoder.h"
#include "video_core.h"
#include "fifo_player.h"

#include <vector>
using namespace std;

/// Structure for the TGA texture format (for dumping)
struct TGAHeader {
    char  idlength;
    char  colourmaptype;
    char  datatypecode;
    short int colourmaporigin;
    short int colourmaplength;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};

namespace gp {

////////////////////////////////////////////////////////////////////////////////

u8 tmem[TMEM_SIZE];

////////////////////////////////////////////////////////////////////////////////
// TEXTURE FORMAT DECODING

static inline u32 __decode_col_rgb5a3(u16 _data) {
    u8 r, g, b, a;

    if (_data & SIGNED_BIT16) // rgb5
    {
        r = (u8)(255.0f * (((_data >> 10) & 0x1f) / 32.0f));
        g = (u8)(255.0f * (((_data >> 5) & 0x1f) / 32.0f));
        b = (u8)(255.0f * ((_data & 0x1f) / 32.0f));
        a = 0xff;	
    }else{ // rgb4a3
        r = 17 * ((_data >> 8) & 0xf);
        g = 17 * ((_data >> 4) & 0xf);
        b = 17 * (_data & 0xf);
        a = (u8)(255.0f * (((_data >> 12) & 7) / 8.0f));
    }

    return (a << 24) | (b << 16) | (g << 8) | r;
}

static inline u32 __decode_col_rgb565(u16 _data) {
    u8 r, g, b;

    // unpack colors
    r = (_data >> 11) << 3;
    g = ((_data >> 5) & 0x3f) << 2;
    b = (_data & 0x1f)<< 3;

    return 0xff000000 | (b << 16) | (g << 8) | (r);
}

void DumpTextureTGA(char filename[], u16 width, u16 height, u8* data) {
    TGAHeader hdr;
    FILE* fout;
    u8 r, g, b;

    memset(&hdr, 0, sizeof(hdr));
    hdr.datatypecode = 2; // uncompressed RGB
    hdr.bitsperpixel = 24; // 24 bpp
    hdr.width = width;
    hdr.height = height;

    fout = fopen(filename, "wb");

    fwrite(&hdr, sizeof(TGAHeader), 1, fout);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            r = data[(4 * (i * width)) + (4 * j) + 0];
            g = data[(4 * (i * width)) + (4 * j) + 1];
            b = data[(4 * (i * width)) + (4 * j) + 2];
            putc(b, fout);
            putc(g, fout);
            putc(r, fout);
        }
    }
    fclose(fout);
}

static inline void DecodeDtxBlock(const u8 *_src, u32 *_dst, u32 _width) {
    u16 color1 = *(u16*)(_src + 2);
    u16 color2 = *(u16*)(_src);
    u32 bits = *(u32*)(_src + 4);

    // Prepare color table
    u32 table[4];
    table[0] = __decode_col_rgb565(color1);
    table[1] = __decode_col_rgb565(color2);

    if (color1 > color2) {
        table[2] =
            ((((2*((table[0] >>  0) & 0xff) + ((table[1] >>  0) & 0xff))/3) & 0xFF) <<  0) |    // R
            ((((2*((table[0] >>  8) & 0xff) + ((table[1] >>  8) & 0xff))/3) & 0xFF) <<  8) |    // G
            ((((2*((table[0] >> 16) & 0xff) + ((table[1] >> 16) & 0xff))/3) & 0xFF) << 16) |    // B
            0xff000000;

        table[3] =
            ((((2*((table[1] >>  0) & 0xff) + ((table[0] >>  0) & 0xff))/3) & 0xFF) <<  0) |    // R
            ((((2*((table[1] >>  8) & 0xff) + ((table[0] >>  8) & 0xff))/3) & 0xFF) <<  8) |    // G
            ((((2*((table[1] >> 16) & 0xff) + ((table[0] >> 16) & 0xff))/3) & 0xFF) << 16) |    // B
            0xff000000;
    } else {
        table[2] =
            ((((((table[0] >>  0) & 0xff) + ((table[1] >>  0) & 0xff)) / 2) & 0xFF) <<  0) |    // R
            ((((((table[0] >>  8) & 0xff) + ((table[1] >>  8) & 0xff)) / 2) & 0xFF) <<  8) |    // G
            ((((((table[0] >> 16) & 0xff) + ((table[1] >> 16) & 0xff)) / 2) & 0xFF) << 16) |    // B
            0xff000000;

        table[3] = 0x00000000;		//alpha
    }
    // Decode image 4x4 layout
    for (int iy = 3; iy >= 0; iy--) {
        _dst[(iy * _width) + 0] = table[(bits >> 6) & 0x3];
        _dst[(iy * _width) + 1] = table[(bits >> 4) & 0x3];
        _dst[(iy * _width) + 2] = table[(bits >> 2) & 0x3];
        _dst[(iy * _width) + 3] = table[(bits >> 0) & 0x3];
        bits >>= 8;
    }
}

static inline void DecompressDxt1(u32* _dst, const u8* _src, int _width, int _height) {
    u8*	runner = (u8 *)_src;

    //#pragma omp for ordered schedule(dynamic)
    for (int y = 0; y < _height; y += 8) {
        for (int x = 0; x < _width; x += 8) {

            //#pragma omp ordered
            DecodeDtxBlock(runner, &_dst[(y*_width)+x], _width);
            runner += 8;
            DecodeDtxBlock(runner, &_dst[(y*_width)+x+4], _width);
            runner += 8;
            DecodeDtxBlock(runner, &_dst[((y+4)*_width)+x], _width);
            runner += 8;
            DecodeDtxBlock(runner, &_dst[((y+4)*_width)+x+4], _width);
            runner += 8;
        }
    }
}

void unpackPixel(int _idx, u8* _dst, u16* _palette, u8 _fmt) {
    //note, the _palette pointer is to Mem_RAM which is
    //already swapped around for normal 32bit reads
    //we have to swap which of the 16bit entries we read
    _idx ^= 1;

    switch(_fmt) {
    case 0: // ia8
        _dst[0] = _palette[_idx] >> 8; //& 0xFF;
        _dst[1] = _palette[_idx] >> 8; //& 0xFF;
        _dst[2] = _palette[_idx] >> 8; //& 0xFF;
        _dst[3] = _palette[_idx] & 0xFF;
        break;

    case 1: // rgb565

        *((u32*)_dst) = __decode_col_rgb565(_palette[_idx]);
        break;

    case 2: // rgb5a3

        *((u32*)_dst) = __decode_col_rgb5a3(_palette[_idx]);
        break;
    }
}

void unpack8(u8* dst, u8* src, int w, int h, u16* palette, u8 paletteFormat, int neww)
{
    u8* runner = dst;

    //printf("fmt: %d", paletteFormat);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < neww; ++x, runner += 4)
            unpackPixel(src[y*w + x], runner, palette, paletteFormat);
}


void DecodeTexture(u8 format, u32 hash, u32 addr, u16 height, u16 width) {
    int	x, y, dx, dy, i = 0, w = width, h = height, j = 0, original_width = width;
    u32 val;

    if (!addr) return;

    u8	*dst8 = (u8*)malloc(w * h * 32);
    u8	*tmp = (u8*)malloc(w * h * 32);
    u8	*tmpptr = tmp;
    u8	*dst = (u8*)malloc(w * h * 32); //final texture (power of two)
    u16 *dst16 = (u16*)dst8;
    u32 *dst32 = (u32*)dst8;
    int temp1=0, temp2=0;

    u8	*src8 = ((u8*)(&Mem_RAM[addr & RAM_MASK]));
    u16	*src16 = ((u16*)(&Mem_RAM[addr & RAM_MASK]));
    u32	*src32 = ((u32*)(&Mem_RAM[addr & RAM_MASK]));

    if (fifo_player::IsRecording()) {
        fifo_player::MemUpdate(addr, src8, w * h * 4); // TODO: Use proper size!
    }

    u8 pallette_fmt = (gp::g_bp_regs.mem[0x98] >> 10) & 3;
    u32 pallette_addr = ((gp::g_bp_regs.mem[0x98] & 0x3ff) << 5);
    u16	*pal16 = ((u16*)&gp::tmem[pallette_addr & TMEM_MASK]);

    switch(format) {
    case 0: // i4
        //multiple of 8 for width

        width = (original_width + 7) & ~7;

        //#pragma omp for ordered schedule(dynamic)
        for (y = 0; y < height; y += 8) {
            for (x = 0; x < width; x += 8) {
                for (dy = 0; dy < 8; dy++) {
                    for (dx = 0; dx < 8; dx+=2, src8++) {
                        //#pragma omp ordered
                        // This is correct - Converts 2 4-bit instensity pixels to 32-bit RGBA
						val = ((*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f)) * 0x11111111;
						dst32[(width * (y + dy) + x + dx + 1)] = val;
						val = ((*(u8 *)((uintptr_t)src8 ^ 3) & 0xf0) >> 4) * 0x11111111;
						dst32[(width * (y + dy) + x + dx)] = val;
 					}
                }
            }
        }
        for (y = 0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
        video_core::g_renderer->AddTexture(w, h, hash, tmp);
        break;

    case 1: // i8
        //multiple of 8 for width
        width = (original_width + 7) & ~7;

        //#pragma omp for ordered schedule(dynamic)
        for (y = 0; y < height; y += 4) {
            for (x = 0; x < original_width; x += 8) {
                for (dy = 0; dy < 4; dy++) {
                    for (dx = 0; dx < 8; dx++) {
                        //#pragma omp ordered
                        // This is correct - Converts one 8-bit instensity pixel to 32-bit RGBA
						val = (*(u8 *)((uintptr_t)src8 ^ 3)) * 0x1010101;
						dst32[(width * (y + dy) + x + dx)] = val;
                        src8++;
					}
                }
            }
        }
        for (y=0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
        video_core::g_renderer->AddTexture(w, h, hash, tmp);
        break;

    case 2: // ia4
        //multiple of 8 for width
        width = (original_width + 7) & ~7;

        //#pragma omp for ordered schedule(dynamic)
        for (y = 0; y < height; y += 4) {
            for (x = 0; x < width; x += 8) {
                for (dy = 0; dy < 4; dy++) {
                    for (dx = 0; dx < 8; dx++, src8++) {
                        //#pragma omp ordered
		 				val = (((*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f)) * 0x111111) | 
                            ((17 * ((*(u8 *)((uintptr_t)src8 ^ 3) & 0xf0) >> 4)) << 24);
		 				dst32[(width * (y + dy) + x + dx)] = val;
 		 			}
                }
            }
        }
        for (y=0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
		video_core::g_renderer->AddTexture(original_width, height, hash, tmp);
		break;

    case 3: // ia8
        //multiple of 4 for width
        width = (original_width + 3) & ~3;

        //#pragma omp for ordered schedule(dynamic)
        for (y = 0; y < height; y += 4) {
            for (x = 0; x < width; x += 4) {
                for (dy = 0; dy < 4; dy++) {
                    for (dx = 0; dx < 4; dx++, src8+=2) {
                        //#pragma omp ordered
						val = (((u32)*(u8 *)(((uintptr_t)src8 + 1) ^ 3)) * 0x00010101) | 
                            ((u32)*(u8 *)((uintptr_t)src8 ^ 3) << 24);
						dst32[(width * (y + dy) + x + dx)] = val;
					}
                }
            }
        }
        for (y=0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
        video_core::g_renderer->AddTexture(w, h, hash, tmp);
		break;

    case 4: // rgb565
		j=0;
		width = (original_width + 3) & ~3;

        //#pragma omp for ordered schedule(dynamic)
		for (y = 0; y < height; y += 4) {
			for (x = 0; x < width; x += 4) {
				for (dy = 0; dy < 4; dy++) {
					for (dx = 0; dx < 4; dx++) {
                        //#pragma omp ordered
						// memory is not already swapped.. use this to grab high word first
						j ^= 1; 
						// decode color
						dst32[width * (y + dy) + x + dx] = __decode_col_rgb565((*((u16*)(src16 + j))));
						// only increment every other time otherwise you get address doubles
						if (!j) src16 += 2; 
					}
                }
            }
        }
        for (y=0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
        video_core::g_renderer->AddTexture(w, h, hash, tmp);
        break;

    case 5: // rgb5a3
		j=0;
		width = (original_width + 3) & ~3;

        //#pragma omp for ordered schedule(dynamic)
		for (y = 0; y < height; y += 4) {
			for (x = 0; x < width; x += 4) {
				for (dy = 0; dy < 4; dy++) {
					for (dx = 0; dx < 4; dx++) {
                        //#pragma omp ordered
						// memory is not already swapped.. use this to grab high word first
						j ^= 1;
						// decode color
						dst32[width * (y + dy) + x + dx] = __decode_col_rgb5a3((*((u16*)(src16 + j))));
						// only increment every other time otherwise you get address doubles
						if (!j) src16 += 2;
					}
                }
            }
        }
		for (y=0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
		video_core::g_renderer->AddTexture(w, h, hash, tmp);
		break;

    case 6: // rgba8
		j=0;
		width = (original_width + 3) & ~3;

        //#pragma omp for ordered schedule(dynamic)
		for (y = 0; y < height; y += 4) {
			for (x = 0; x < width; x += 4) {
				for (dy = 0; dy < 4; dy++) {
					for (dx = 0; dx < 4; dx++) {
                        //#pragma omp ordered
						// memory is not already swapped.. use this to grab high word first
						j ^= 1;
						// fetch data
						dst32[width * (y + dy) + x + dx] = ((*((u16*)(src16 + j))) << 16);
						// only increment every other time otherwise you get address doubles
						if (!j) src16 += 2;
					}
                }
				for (dy = 0; dy < 4; dy++) {
					for (dx = 0; dx < 4; dx++) {
                        //#pragma omp ordered
						// memory is not already swapped.. use this to grab high word first
						j ^= 1;
						// fetch color data
						u32 data = dst32[width * (y + dy) + x + dx] | ((*((u16*)(src16 + j))));
						// decode color data
						dst32[width * (y + dy) + x + dx] = (data & 0xff00ff00) | 
										((data & 0xff0000) >> 16) | 
										((data & 0xff) << 16);
						// only increment every other time otherwise you get address doubles
						if (!j) src16 += 2;
					}
                }
			}
        }
		for (y=0; y < height; y++) {
            memcpy(&tmp[y*original_width*4], &dst8[y*width*4], original_width*4);
        }
        video_core::g_renderer->AddTexture(original_width, height, hash, tmp);
		break;

    case 8: // c4
    case 9: // c8
        {
            int _width = width;
            switch(format) {
            case 8:
                width = (_width + 7) & ~7;
                //#pragma omp for ordered schedule(dynamic)
                for (y = 0; y < height; y += 8) {
                    for (x = 0; x < width; x += 8) {
                        //#pragma omp parallel for
                        for (dy = 0; dy < 8; dy++) {
                            //#pragma omp ordered
                            dst8[width * (y + dy) + x + 0] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
                            dst8[width * (y + dy) + x + 1] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
                            src8++;

                            dst8[width * (y + dy) + x + 2] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
                            dst8[width * (y + dy) + x + 3] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
                            src8++;

                            dst8[width * (y + dy) + x + 4] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
                            dst8[width * (y + dy) + x + 5] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
                            src8++;

                            dst8[width * (y + dy) + x + 6] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
                            dst8[width * (y + dy) + x + 7] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
                            src8++;
                        }
                    }
                }
                break;

            case 9:
                width = (_width + 7) & ~7;
                //#pragma omp for ordered schedule(dynamic)
                for (y = 0; y < height; y += 4) {
                    //#pragma omp parallel for
                    for (x = 0; x < width; x += 8) {

                        //#pragma omp ordered
                        *(u32 *)&dst8[width * (y + 0) + x] = BSWAP32(*(u32 *)((uintptr_t)src8));
                        *(u32 *)&dst8[width * (y + 0) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 4));

                        *(u32 *)&dst8[width * (y + 1) + x] = BSWAP32(*(u32 *)((uintptr_t)src8 + 8));
                        *(u32 *)&dst8[width * (y + 1) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 12));

                        *(u32 *)&dst8[width * (y + 2) + x] = BSWAP32(*(u32 *)((uintptr_t)src8 + 16));
                        *(u32 *)&dst8[width * (y + 2) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 20));

                        *(u32 *)&dst8[width * (y + 3) + x] = BSWAP32(*(u32 *)((uintptr_t)src8 + 24));
                        *(u32 *)&dst8[width * (y + 3) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 28));
                        src8+=32;
                    }
                }
                break;
            }
            unpack8(tmp, dst8, width, height, pal16, pallette_fmt, _width);

            video_core::g_renderer->AddTexture(w, h, hash, tmp);
        }
        break;

    case 14:
        width = (width + 7) & ~7;
        DecompressDxt1(dst32, src8, width, height);

        for (y=0; y < height; y++) {
            memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);
        }
        video_core::g_renderer->AddTexture(w, h, hash, tmp);
        break;

    default:
        LOG_ERROR(TGP, "Unsupported texture format %d!", format);
        break;
    }

    if (common::g_config->current_renderer_config().enable_texture_dumping) {
        char filepath[MAX_PATH], filename[MAX_PATH];
        strcpy(filepath, common::g_config->program_dir());
        strcat(filepath, "dump");
        mkdir(filepath);
        strcat(filepath, "/textures");
        mkdir(filepath);
        sprintf(filename, "/%d.tga", addr);
        strcat(filepath, filename);
        DumpTextureTGA(filepath, w, h, tmp);
    }

    // free manually allocated memory
    free(dst8);
    free(tmp);
    free(dst);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

} // namespace
