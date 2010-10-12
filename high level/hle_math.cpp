// hle_math.cpp
// (c) 2005,2006 Gekko Team

#include "../emu.h"
#include "hle_math.h"
#include "../cpu core/cpu_core_regs.h"
/*
Mtx mTmp[4];
Vec vTmp[4];

////////////////////////////////////////////////////////////

static inline void MTXCopy(void *src, void *dst)
{
	memcpy (dst, src, 4 * 12);
}

static inline void VECCopy(void *src, void *dst)
{
	memcpy (dst, src, 4 * 3);
}

static inline void BSwapData(u32 *addr, s32 count)
{
    u32 *until = addr + count / sizeof(u32);

    while(addr != until)
    {
        *addr = BSWAP32(*addr);
        addr++;
    }
}

static inline void MTXRotTrig(MtxPtr _m, u32 _ax, f32 _sin, f32 _cos)
{
    switch(_ax)
    {
    case 'x':
    case 'X':
        _m->_f32[0][0] =  1.0f;  _m->_f32[0][1] =  0.0f;    _m->_f32[0][2] =  0.0f;  _m->_f32[0][3] = 0.0f;
        _m->_f32[1][0] =  0.0f;  _m->_f32[1][1] =  _cos;    _m->_f32[1][2] = -_sin;  _m->_f32[1][3] = 0.0f;
        _m->_f32[2][0] =  0.0f;  _m->_f32[2][1] =  _sin;    _m->_f32[2][2] =  _cos;  _m->_f32[2][3] = 0.0f;
        break;
    case 'y':
    case 'Y':
        _m->_f32[0][0] =  _cos;  _m->_f32[0][1] =  0.0f;    _m->_f32[0][2] =  _sin;  _m->_f32[0][3] = 0.0f;
        _m->_f32[1][0] =  0.0f;  _m->_f32[1][1] =  1.0f;    _m->_f32[1][2] =  0.0f;  _m->_f32[1][3] = 0.0f;
        _m->_f32[2][0] = -_sin;  _m->_f32[2][1] =  0.0f;    _m->_f32[2][2] =  _cos;  _m->_f32[2][3] = 0.0f;
        break;
    case 'z':
    case 'Z':
        _m->_f32[0][0] =  _cos;  _m->_f32[0][1] = -_sin;    _m->_f32[0][2] =  0.0f;  _m->_f32[0][3] = 0.0f;
        _m->_f32[1][0] =  _sin;  _m->_f32[1][1] =  _cos;    _m->_f32[1][2] =  0.0f;  _m->_f32[1][3] = 0.0f;
        _m->_f32[2][0] =  0.0f;  _m->_f32[2][1] =  0.0f;    _m->_f32[2][2] =  1.0f;  _m->_f32[2][3] = 0.0f;
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////

HLE(PSMTXInverse)
{
	MtxPtr src = (MtxPtr)(&Mem_RAM[GPR(3) & RAM_MASK]);
	MtxPtr inv = (MtxPtr)(&Mem_RAM[GPR(4) & RAM_MASK]);
    MtxPtr m;
    f32 det;

//    BSwapData((u32*)src, SIZE_OF_MTX3X4);
//    BSwapData((u32*)inv, SIZE_OF_MTX3X4);

    if( src == inv )
    {
        m = mTmp;
    }else{
        m = inv;
    }

    det =   src->_f32[0][0]*src->_f32[1][1]*src->_f32[2][2] + src->_f32[0][1]*src->_f32[1][2]*src->_f32[2][0] + src->_f32[0][2]*src->_f32[1][0]*src->_f32[2][1]
          - src->_f32[2][0]*src->_f32[1][1]*src->_f32[0][2] - src->_f32[1][0]*src->_f32[0][1]*src->_f32[2][2] - src->_f32[0][0]*src->_f32[2][1]*src->_f32[1][2];

    if( det == 0.0f )
    {
        GPR(5) = 0;
		return;
    }

    det = 1.0f / det;

    m->_f32[0][0] =  (src->_f32[1][1]*src->_f32[2][2] - src->_f32[2][1]*src->_f32[1][2]) * det;
    m->_f32[0][1] = -(src->_f32[0][1]*src->_f32[2][2] - src->_f32[2][1]*src->_f32[0][2]) * det;
    m->_f32[0][2] =  (src->_f32[0][1]*src->_f32[1][2] - src->_f32[1][1]*src->_f32[0][2]) * det;

    m->_f32[1][0] = -(src->_f32[1][0]*src->_f32[2][2] - src->_f32[2][0]*src->_f32[1][2]) * det;
    m->_f32[1][1] =  (src->_f32[0][0]*src->_f32[2][2] - src->_f32[2][0]*src->_f32[0][2]) * det;
    m->_f32[1][2] = -(src->_f32[0][0]*src->_f32[1][2] - src->_f32[1][0]*src->_f32[0][2]) * det;

    m->_f32[2][0] =  (src->_f32[1][0]*src->_f32[2][1] - src->_f32[2][0]*src->_f32[1][1]) * det;
    m->_f32[2][1] = -(src->_f32[0][0]*src->_f32[2][1] - src->_f32[2][0]*src->_f32[0][1]) * det;
    m->_f32[2][2] =  (src->_f32[0][0]*src->_f32[1][1] - src->_f32[1][0]*src->_f32[0][1]) * det;

    m->_f32[0][3] = -m->_f32[0][0]*src->_f32[0][3] - m->_f32[0][1]*src->_f32[1][3] - m->_f32[0][2]*src->_f32[2][3];
    m->_f32[1][3] = -m->_f32[1][0]*src->_f32[0][3] - m->_f32[1][1]*src->_f32[1][3] - m->_f32[1][2]*src->_f32[2][3];
    m->_f32[2][3] = -m->_f32[2][0]*src->_f32[0][3] - m->_f32[2][1]*src->_f32[1][3] - m->_f32[2][2]*src->_f32[2][3];

    if( m == mTmp )
    {
        MTXCopy( mTmp,inv );
    }

//    BSwapData((u32*)src, SIZE_OF_MTX3X4);
//    BSwapData((u32*)inv, SIZE_OF_MTX3X4);
//	BSwapData((u32*)m, SIZE_OF_MTX3X4);

    GPR(5) = 1;
	return;
}

HLE(PSMTXScale)
{
	MtxPtr m = (MtxPtr)(&RAM[GPR(3) & RAM_MASK]);

    BSwapData((u32*)m, SIZE_OF_MTX3X4);

    m->_f32[0][0] = PS0(1);  m->_f32[0][1] = 0.0f;	m->_f32[0][2] = 0.0f; m->_f32[0][3] = 0.0f;
    m->_f32[1][0] = 0.0f; m->_f32[1][1] = PS0(2);	m->_f32[1][2] = 0.0f; m->_f32[1][3] = 0.0f;
    m->_f32[2][0] = 0.0f; m->_f32[2][1] = 0.0f;	m->_f32[2][2] = PS0(3);  m->_f32[2][3] = 0.0f;

    BSwapData((u32*)m, SIZE_OF_MTX3X4);
}

HLE(PSMTXTrans)
{
	MtxPtr m = (MtxPtr)(&RAM[GPR(3) & RAM_MASK]);

    BSwapData((u32*)m, SIZE_OF_MTX3X4);

    m->_f32[0][0] = 1.0f; m->_f32[0][1] = 0.0f; m->_f32[0][2] = 0.0f; m->_f32[0][3] =  PS0(1);
    m->_f32[1][0] = 0.0f; m->_f32[1][1] = 1.0f; m->_f32[1][2] = 0.0f; m->_f32[1][3] =  PS0(2);
    m->_f32[2][0] = 0.0f; m->_f32[2][1] = 0.0f; m->_f32[2][2] = 1.0f; m->_f32[2][3] =  PS0(3);

    BSwapData((u32*)m, SIZE_OF_MTX3X4);
}

HLE(PSMTXRotRad)
{
	MtxPtr m = (MtxPtr)(&RAM[GPR(3) & RAM_MASK]);
    f32 sinA, cosA;

    BSwapData((u32*)m, SIZE_OF_MTX3X4);

    sinA = (f32) sinf((f32) PS0(1));
    cosA = (f32) cosf((f32) PS0(1));

    MTXRotTrig( m, GPR(4), sinA, cosA );

	BSwapData((u32*)m, SIZE_OF_MTX3X4);
}

HLE(PSMTXConcat)
{
	MtxPtr mA = (MtxPtr)(&RAM[GPR(3) & RAM_MASK]);
	MtxPtr mB = (MtxPtr)(&RAM[GPR(4) & RAM_MASK]);
	MtxPtr mAB = (MtxPtr)(&RAM[GPR(5) & RAM_MASK]);
	MtxPtr tmp = (MtxPtr)(&mTmp[0]);
	MtxPtr m;

    if((mAB == mA) || (mAB == mB))
    {
        m = mTmp;
    }else{
        m = mAB;
    }

	BSwapData((u32*)mA, SIZE_OF_MTX3X4);
	BSwapData((u32*)mB, SIZE_OF_MTX3X4);

    m->_f32[0][0] = mA->_f32[0][0]*mB->_f32[0][0] + mA->_f32[0][1]*mB->_f32[1][0] + mA->_f32[0][2]*mB->_f32[2][0];
    m->_f32[0][1] = mA->_f32[0][0]*mB->_f32[0][1] + mA->_f32[0][1]*mB->_f32[1][1] + mA->_f32[0][2]*mB->_f32[2][1];
    m->_f32[0][2] = mA->_f32[0][0]*mB->_f32[0][2] + mA->_f32[0][1]*mB->_f32[1][2] + mA->_f32[0][2]*mB->_f32[2][2];
    m->_f32[0][3] = mA->_f32[0][0]*mB->_f32[0][3] + mA->_f32[0][1]*mB->_f32[1][3] + mA->_f32[0][2]*mB->_f32[2][3] + mA->_f32[0][3];
																							 		 	
    m->_f32[1][0] = mA->_f32[1][0]*mB->_f32[0][0] + mA->_f32[1][1]*mB->_f32[1][0] + mA->_f32[1][2]*mB->_f32[2][0];
    m->_f32[1][1] = mA->_f32[1][0]*mB->_f32[0][1] + mA->_f32[1][1]*mB->_f32[1][1] + mA->_f32[1][2]*mB->_f32[2][1];
    m->_f32[1][2] = mA->_f32[1][0]*mB->_f32[0][2] + mA->_f32[1][1]*mB->_f32[1][2] + mA->_f32[1][2]*mB->_f32[2][2];
    m->_f32[1][3] = mA->_f32[1][0]*mB->_f32[0][3] + mA->_f32[1][1]*mB->_f32[1][3] + mA->_f32[1][2]*mB->_f32[2][3] + mA->_f32[1][3];
																									 	
    m->_f32[2][0] = mA->_f32[2][0]*mB->_f32[0][0] + mA->_f32[2][1]*mB->_f32[1][0] + mA->_f32[2][2]*mB->_f32[2][0];
    m->_f32[2][1] = mA->_f32[2][0]*mB->_f32[0][1] + mA->_f32[2][1]*mB->_f32[1][1] + mA->_f32[2][2]*mB->_f32[2][1];
    m->_f32[2][2] = mA->_f32[2][0]*mB->_f32[0][2] + mA->_f32[2][1]*mB->_f32[1][2] + mA->_f32[2][2]*mB->_f32[2][2];
    m->_f32[2][3] = mA->_f32[2][0]*mB->_f32[0][3] + mA->_f32[2][1]*mB->_f32[1][3] + mA->_f32[2][2]*mB->_f32[2][3] + mA->_f32[2][3];

	BSwapData((u32*)mA, SIZE_OF_MTX3X4);
	BSwapData((u32*)mB, SIZE_OF_MTX3X4);
	BSwapData((u32*)m, SIZE_OF_MTX3X4);

    if(m == mTmp)
    {
        MTXCopy( mTmp, mAB );
    }
}
*/