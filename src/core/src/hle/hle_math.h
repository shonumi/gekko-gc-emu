// hle_math.h
// (c) 2005,2006 Gekko Team

#ifndef _HLE_SDK_MTX_H_
#define _HLE_SDK_MTX_H_

////////////////////////////////////////////////////////////

#define	SIZE_OF_MTX3X4			(3 * 4 * 4)
#define	SIZE_OF_VEC				(3 * 4)
#define	SIZE_OF_FLOAT			(4)

////////////////////////////////////////////////////////////
// Desc: Matrix/Vector Types
////////////////////////////////////////////////////////////

// 3D Vector, point
typedef union
{
	f32 _f32[3];
	u32 _u32[3];
} Vec, *VecPtr, Point3d, *Point3dPtr;

// Signed 16bit 3D vector
typedef struct
{   
    s16 x;
    s16 y;
    s16 z;
}S16Vec, *S16VecPtr;

// Quaternion
typedef struct
{
	f32 x, y, z, w;
} Quaternion, *QuaternionPtr, Qtrn, *QtrnPtr;

// 3x4 Matrix, pointer
typedef union
{
	f32 _f32[3][4];
	u32 _u32[3][4];
} Mtx, *MtxPtr;

// 4x3 Reordered matrix, pointer
typedef f32 ROMtx[4][3];
typedef f32 (*ROMtxPtr)[3];

// 4x4 Matrix , pointer
// used for projection matrix
typedef f32 Mtx44[4][4];
typedef f32 (*Mtx44Ptr)[4];

////////////////////////////////////////////////////////////

#endif

