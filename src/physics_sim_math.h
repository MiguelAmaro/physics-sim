/* date = September 20th 2021 7:46 pm */

#ifndef ACC_ARDUINO_MATH_H
#define ACC_ARDUINO_MATH_H

#include "physics_sim_types.h"

union mat2f32
{
    f32 e[4];
};
//
//internal void
//mat2f32_Scale(mat2f32)
//{
//
//return;
//}
//

union v2f32
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 e[2];
};

union v2s32
{
    struct
    {
        s32 x;
        s32 y;
    };
    s32 e[2];
};

union v3f32
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };
    f32 e[3];
};


v3f32 v3f32Init(f32 x, f32 y, f32 z)
{
    v3f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

v3f32 operator *(v3f32 A, f32 Scalar)
{
    v3f32 Result = { 0 };
    
    Result.x = A.x * Scalar;
    Result.y = A.y * Scalar;
    Result.z = A.z * Scalar;
    
    return Result;
}

v3f32 operator *(f32 Scalar, v3f32 A)
{
    v3f32 Result = A * Scalar;
    
    return Result;
}

//
//v3f32 *operator *=(v3f32 *A, f32 Scalar)
//{
//v3f32 *Result = A;
//
//Result->x = A->x * Scalar;
//Result->y = A->y * Scalar;
//Result->z = A->z * Scalar;
//
//return Result;
//}
//

union v4f32
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 e[4];
};


v4f32 v4f32Init(f32 x, f32 y, f32 z, f32 w)
{
    v4f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.z = w;
    
    return Result;
}


union rect_v2f32
{
    struct
    {
        v2f32 min;
        v2f32 max;
    };
    f32 e[2];
};

union rect_v2s32
{
    struct
    {
        v2s32 min;
        v2s32 max;
    };
    s32 e[4];
};


#endif //ACC_ARDUINO_MATH_H
