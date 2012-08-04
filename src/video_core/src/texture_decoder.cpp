// gx_texture.cpp
// (c) 2005,2009 Gekko Team

#include "common.h"
#include "config.h"
#include "memory.h"
#include "hw/hw_gx.h"
#include "texture_decoder.h"
#include "renderer_gl3/renderer_gl3.h"

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

// cache
GLuint texcache[TMEM_SIZE];

////////////////////////////////////////////////////////////////////////////////
// TEXTURE FORMAT DECODING

static inline u32 __decode_col_rgb5a3(u16 _data) {
    u8 r, g, b, a;

    if(_data & SIGNED_BIT16) // rgb5
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

    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
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

    for(int y = 0; y < _height; y += 8) {
        for(int x = 0; x < _width; x += 8) {
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
    for(int y = 0; y < h; ++y)
        for(int x = 0; x < neww; ++x, runner += 4)
            unpackPixel(src[y*w + x], runner, palette, paletteFormat);
}


void DecodeTexture(u8 format, u32 addr, u16 height, u16 width) {
    int	x, y, dx, dy, i = 0, w = width, h = height, j = 0;
    u32 val;

    if(!addr) return;

    //	while(w < width) w *= 2; // get power of two width
    //	while(h < height) h *= 2; // .. height

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

    //if(addr & 3)
    //    printf("Texture at %08X not boundary aligned!\n", addr);

    glGenTextures(1, &texcache[TEX_CACHE_LOCATION(addr >> 5)]);
    glBindTexture(GL_TEXTURE_2D, texcache[TEX_CACHE_LOCATION(addr >> 5)]);
    /*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    */
    //if(num < 4) 
    //    gx_states::tx_setmode0(0x80 + num); 
    //else 
    //    gx_states::tx_setmode0(0xa0 + (_tx.num - 4));

    u8 pallette_fmt = (bp.mem[0x98] >> 10) & 3;
    u32 pallette_addr = ((bp.mem[0x98] & 0x3ff) << 5);
    u16	*pal16 = ((u16*)PTEXTURE_MEM(pallette_addr));

    //printf("TEXTURE %d Width %d Height %d\n", _tx.fmt, _tx.width, _tx.height);

    switch(format) {
    case 0: // i4
        //multiple of 8 for width
        width = (width + 7) & ~7;
        for(y = 0; y < height; y += 8)
            for(x = 0; x < width; x += 8)
                for(dy = 0; dy < 8; dy++)
                    for(dx = 0; dx < 8; dx+=2, src8++)
                    {
                        val = ((17 * (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f)) * 0x00010101) | 0xFF000000;
                        dst32[(width * (y + dy) + x + dx + 1)] = val;
                        val = ((17 * (*(u8 *)((uintptr_t)src8 ^ 3) & 0xf0) >> 4) * 0x00010101) | 0xFF000000;
                        dst32[(width * (y + dy) + x + dx)] = val;
                        /*
                        dst8[(width * (y + dy) + x + dx + 1) * 4] = (17 * (*(u8 *)((u32)src8 ^ 3) & 0xf));
                        dst8[(width * (y + dy) + x + dx + 1) * 4 + 1] = (17 * (*(u8 *)((u32)src8 ^ 3) & 0xf));
                        dst8[(width * (y + dy) + x + dx + 1) * 4 + 2] = (17 * (*(u8 *)((u32)src8 ^ 3) & 0xf));
                        dst8[(width * (y + dy) + x + dx + 1) * 4 + 3] = 0xFF;
                        dst8[(width * (y + dy) + x + dx) * 4 + 1] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0xf0) >> 4));
                        dst8[(width * (y + dy) + x + dx) * 4] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0xf0) >> 4));
                        dst8[(width * (y + dy) + x + dx) * 4 + 2] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0xf0) >> 4));
                        dst8[(width * (y + dy) + x + dx) * 4 + 3] = 0xFF;
                        */
                    }

                    for(y=0; y < height; y++)
                        memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

                    //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
                    break;

    case 1: // i8
        //multiple of 8 for width
        width = (width + 7) & ~7;

        for(y = 0; y < height; y += 4)
            for(x = 0; x < width; x += 8)
                for(dy = 0; dy < 4; dy++)
                    for(dx = 0; dx < 8; dx++)
                    {
                        val = ((*(u8 *)((uintptr_t)src8 ^ 3)) * 0x00010101) | 0xFF000000;
                        dst32[(width * (y + dy) + x + dx)] = val;
                        /*
                        dst8[(width * (y + dy) + x + dx) * 4] = *(u8 *)((u32)src8 ^ 3);
                        dst8[(width * (y + dy) + x + dx) * 4+1] = *(u8 *)((u32)src8 ^ 3);
                        dst8[(width * (y + dy) + x + dx) * 4+2] = *(u8 *)((u32)src8 ^ 3);
                        dst8[(width * (y + dy) + x + dx) * 4+3] = 0xFF;
                        */
                        src8++;
                    }

                    for(y=0; y < height; y++)
                        memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

                    //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);

                    break; 

    case 2: // ia4
        //multiple of 8 for width
        width = (width + 7) & ~7;
        for(y = 0; y < height; y += 4)
            for(x = 0; x < width; x += 8)
                for(dy = 0; dy < 4; dy++)
                    for(dx = 0; dx < 8; dx++, src8++)
                    {
                        val = ((17 * ((*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f))) * 0x00010101) | ((17 * ((*(u8 *)((uintptr_t)src8 ^ 3) & 0xf0) >> 4)) << 24);
                        dst32[(width * (y + dy) + x + dx)] = val;

                        /*
                        dst8[(width * (y + dy) + x + dx) * 4] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0x0f)));
                        dst8[(width * (y + dy) + x + dx) * 4 + 1] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0x0f)));
                        dst8[(width * (y + dy) + x + dx) * 4 + 2] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0x0f)));
                        dst8[(width * (y + dy) + x + dx) * 4 + 3] = (17 * ((*(u8 *)((u32)src8 ^ 3) & 0xf0) >> 4));
                        */
                    }

                    for(y=0; y < height; y++)
                        memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

                    gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);

                    break;

    case 3: // ia8
        //multiple of 4 for width
        width = (width + 3) & ~3;
        for(y = 0; y < height; y += 4)
            for(x = 0; x < width; x += 4)
                for(dy = 0; dy < 4; dy++)
                    for(dx = 0; dx < 4; dx++, src8+=2)
                    {
                        val = (((u32)*(u8 *)(((uintptr_t)src8 + 1) ^ 3)) * 0x00010101) | ((u32)*(u8 *)((uintptr_t)src8 ^ 3) << 24);
                        dst32[(width * (y + dy) + x + dx)] = val;
                        /*
                        dst8[(width * (y + dy) + x + dx) * 4 + 3] = *(u8 *)((u32)src8 ^ 3);
                        src8++;
                        dst8[(width * (y + dy) + x + dx) * 4] = *(u8 *)((u32)src8 ^ 3);
                        dst8[(width * (y + dy) + x + dx) * 4 + 1] = *(u8 *)((u32)src8 ^ 3);
                        dst8[(width * (y + dy) + x + dx) * 4 + 2] = *(u8 *)((u32)src8 ^ 3);
                        */
                    }

                    for(y=0; y < height; y++)
                        memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

                    //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);

                    break;

    case 4: // rgb565
        j=0;
        width = (width + 3) & ~3;
        for(y = 0; y < height; y += 4)
            for(x = 0; x < width; x += 4)
                for(dy = 0; dy < 4; dy++)
                    for(dx = 0; dx < 4; dx++)
                    {
                        // memory is not already swapped.. use this to grab high word first
                        j ^= 1; 
                        // decode color
                        dst32[width * (y + dy) + x + dx] = __decode_col_rgb565((*((u16*)(src16 + j))));
                        // only increment every other time otherwise you get address doubles
                        if(!j) src16 += 2; 
                    }

                    for(y=0; y < height; y++)
                        memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

                    //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
                    break;

    case 5: // rgb5a3
        j=0;
        width = (width + 3) & ~3;
        for(y = 0; y < height; y += 4)
            for(x = 0; x < width; x += 4)
                for(dy = 0; dy < 4; dy++)
                    for(dx = 0; dx < 4; dx++)
                    {
                        // memory is not already swapped.. use this to grab high word first
                        j ^= 1;
                        // decode color
                        dst32[width * (y + dy) + x + dx] = __decode_col_rgb5a3((*((u16*)(src16 + j))));
                        // only increment every other time otherwise you get address doubles
                        if(!j) src16 += 2;
                    }

                    for(y=0; y < height; y++)
                        memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

                    //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);

                    break;

    case 6: // rgba8
        j=0;
        width = (width + 3) & ~3;
        for(y = 0; y < height; y += 4)
            for(x = 0; x < width; x += 4)
            {
                for(dy = 0; dy < 4; dy++)
                    for(dx = 0; dx < 4; dx++)
                    {
                        // memory is not already swapped.. use this to grab high word first
                        j ^= 1;
                        // fetch data
                        dst32[width * (y + dy) + x + dx] = ((*((u16*)(src16 + j))) << 16);
                        // only increment every other time otherwise you get address doubles
                        if(!j) src16 += 2;
                    }

                    for(dy = 0; dy < 4; dy++)
                        for(dx = 0; dx < 4; dx++)
                        {
                            // memory is not already swapped.. use this to grab high word first
                            j ^= 1;
                            // fetch color data
                            u32 data = dst32[width * (y + dy) + x + dx] | ((*((u16*)(src16 + j))));
                            // decode color data
                            dst32[width * (y + dy) + x + dx] = (data & 0xff00ff00) | 
                                ((data & 0xff0000) >> 16) | 
                                ((data & 0xff) << 16);
                            // only increment every other time otherwise you get address doubles
                            if(!j) src16 += 2;
                        }

            }

            for(y=0; y < height; y++)
                memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

            //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);

            break;

    case 8: // c4
    case 9: // c8
        {
            int width_power_of_2 = 0;
            switch(format)
		    {
            case 8:
			    width_power_of_2 = (width + 7) & ~7;
			    for(y = 0; y < height; y += 8)
				    for(x = 0; x < width_power_of_2; x += 8)
					    for(dy = 0; dy < 8; dy++)
					    {
						    tmp[width_power_of_2 * (y + dy) + x + 0] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
						    tmp[width_power_of_2 * (y + dy) + x + 1] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
						    src8++;

						    tmp[width_power_of_2 * (y + dy) + x + 2] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
						    tmp[width_power_of_2 * (y + dy) + x + 3] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
						    src8++;

						    tmp[width_power_of_2 * (y + dy) + x + 4] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
						    tmp[width_power_of_2 * (y + dy) + x + 5] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
						    src8++;

						    tmp[width_power_of_2 * (y + dy) + x + 6] = (*(u8 *)((uintptr_t)src8 ^ 3) >> 4);
						    tmp[width_power_of_2 * (y + dy) + x + 7] = (*(u8 *)((uintptr_t)src8 ^ 3) & 0x0f);
						    src8++;
					    }

			    break;

            case 9:
			    width = (width + 7) & ~7;

			    for(y = 0; y < height; y += 4)
				    for(x = 0; x < width_power_of_2; x += 8)
				    {
					    *(u32 *)&tmp[width_power_of_2 * (y + 0) + x] = BSWAP32(*(u32 *)((uintptr_t)src8));
					    *(u32 *)&tmp[width_power_of_2 * (y + 0) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 4));

					    *(u32 *)&tmp[width_power_of_2 * (y + 1) + x] = BSWAP32(*(u32 *)((uintptr_t)src8 + 8));
					    *(u32 *)&tmp[width_power_of_2 * (y + 1) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 12));

					    *(u32 *)&tmp[width_power_of_2 * (y + 2) + x] = BSWAP32(*(u32 *)((uintptr_t)src8 + 16));
					    *(u32 *)&tmp[width_power_of_2 * (y + 2) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 20));

					    *(u32 *)&tmp[width_power_of_2 * (y + 3) + x] = BSWAP32(*(u32 *)((uintptr_t)src8 + 24));
					    *(u32 *)&tmp[width_power_of_2 * (y + 3) + x + 4] = BSWAP32(*(u32 *)((uintptr_t)src8 + 28));
					    src8+=32;
				    }
			    break;
		    }

		    unpack8(dst8, tmp, width_power_of_2, height, pal16, pallette_fmt, width);

		    //gluScaleImage(GL_RGBA, _tx.width, _tx.height, GL_UNSIGNED_BYTE, dst8, w, h, GL_UNSIGNED_BYTE, dst);

		    if(pallette_fmt)
		    {
			    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst8);
		    }else{
			    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst8);
		    }
        }
        break;

    case 14:
        width = (width + 7) & ~7;
        DecompressDxt1(dst32, src8, width, height);

        for(y=0; y < height; y++)
            memcpy(&tmp[y*width*4], &dst8[y*width*4], width*4);

        //gluScaleImage(GL_RGBA, width, height, GL_UNSIGNED_BYTE, tmp, w, h, GL_UNSIGNED_BYTE, dst);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp);
        break;

    default:
        LOG_ERROR(TGP, "Unsupported texture format %d!", format);

        return;
    }

    if (common::g_config->current_renderer_config().enable_texture_dumping) {
        char filepath[MAX_PATH], filename[MAX_PATH];
        strcpy_s(filepath, MAX_PATH, common::g_config->program_dir());
        strcat_s(filepath, MAX_PATH, "dump");
        mkdir(filepath);
        strcat_s(filepath, MAX_PATH, "/textures");
        mkdir(filepath);
        sprintf_s(filename, MAX_PATH, "/%d.tga", addr);
        strcat_s(filepath, MAX_PATH, filename);
        DumpTextureTGA(filepath, w, h, tmp);
    }

    // free manually allocated memory
    free(dst8);
    free(tmp);
    free(dst);
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

void TexCacheInit() {
    memset(&texcache, 0, sizeof(texcache));
}

void TexCacheShutdown() {
}

} // namespace
