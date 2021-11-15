/* date = September 20th 2021 7:46 pm */

#ifndef PHYSICS_SIM_MATH_H
#define PHYSICS_SIM_MATH_H

#include "physics_sim_types.h"
#include "thirdparty\sse_mathisfun.h"

#define PI32 (3.141592653589793f)


//~ TRANSENDENTAL FUNCTIONS

f32 Cosine(f32 Radians)
{
    f32 Result = 0.0f;
    
    __m128 Shit = cos_ps(_mm_load_ss(&Radians));
    
    _mm_store_ss(&Result, Shit);
    
    return Result;
};

f32 Sine(f32 Radians)
{
    f32 Result = 0.0f;
    
    __m128 Shit = sin_ps(_mm_load_ss(&Radians));
    
    _mm_store_ss(&Result, Shit);
    
    return Result;
};

//https://stackoverflow.com/questions/18187492/simd-vectorize-atan2-using-arm-neon-assembly?noredirect=1&lq=1
//https://en.wikipedia.org/wiki/Approximation_theory#Chebyshev_approximation
//https://stackoverflow.com/questions/11930594/calculate-atan2-without-std-functions-or-c99
//http://dspguru.com/dsp/books/
f32 ArcTan(f32 Value )
{
    // NOTE(MIGUEL): Uses Rational Approximation.
    //               Better Alternative is Chebyshev Approximation??
    static const u32 SignMask = 0x80000000;
    static const f32 B        = 0.596227f;
    
    // Extract the sign Bit
    u32 ux_s  = SignMask & (u32 &)Value;
    
    // Calculate the arctangent in the first quadrant
    f32 Bx_a = ::fabs( B * Value);
    f32 Num = Bx_a + Value * Value;
    f32 Atan_1q = Num / ( 1.f + Bx_a + Num );
    
    // Restore the sign Bit
    u32 Atan_2q = ux_s | (u32 &)Atan_1q;
    return (f32 &)Atan_2q;
}

float ArcTan2( float y, float x )
{
    static const uint32_t sign_mask = 0x80000000;
    static const float b = 0.596227f;
    
    // Extract the sign bits
    uint32_t ux_s  = sign_mask & (uint32_t &)x;
    uint32_t uy_s  = sign_mask & (uint32_t &)y;
    
    // Determine the quadrant offset
    float q = (float)( ( ~ux_s & uy_s ) >> 29 | ux_s >> 30 ); 
    
    // Calculate the arctangent in the first quadrant
    float bxy_a = ::fabs( b * x * y );
    float num = bxy_a + y * y;
    float atan_1q =  num / ( x * x + bxy_a + num );
    
    // Translate it to the proper quadrant
    uint32_t uatan_2q = (ux_s ^ uy_s) | (uint32_t &)atan_1q;
    return q + (float &)uatan_2q;
} 

f32 Square(f32 Value)
{
    f32 Result = Value * Value;
    
    return Result;
};

f32 Root(f32 Value)
{
    f32 Result = 0.0f;
    
    __m128 Shit = _mm_sqrt_ss(_mm_load_ss(&Value));
    
    _mm_store_ss(&Result, Shit);
    
    return Result;
}


//- VECTORS 

union v2f32
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 c[2];
};

union v2s32
{
    struct
    {
        s32 x;
        s32 y;
    };
    s32 c[2];
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
    struct
    {
        v2f32 xy;
        f32    z;
    };
    f32 c[3];
};

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
    f32 c[4];
};

//- RECTANGLES 

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


//- MATRICES 

union m2f32
{
    v2f32 r[2];
    f32   e[4];
    f32   x[2][2];
};

union m3f32
{
    v3f32 r[3];
    f32   e[9];
    f32   x[3][3];
};


union m4f32
{
    v4f32 r[4];
    f32   e[16];
    f32   x[4][4];
};


//~ OPERATIORS & FUNCTIONS

//- VECTOR 2D 
v2f32 operator +(v2f32 A, v2f32 B)
{
    v2f32 Result = { 0 };
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return Result;
}


v2f32 operator -(v2f32 A, v2f32 B)
{
    v2f32 Result = { 0 };
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return Result;
}


v2f32 operator *(v2f32 A, f32 Scalar)
{
    v2f32 Result = { 0 };
    
    Result.x = A.x * Scalar;
    Result.y = A.y * Scalar;
    
    return Result;
}

v2f32 operator *(f32 Scalar, v2f32 A)
{
    v2f32 Result = A * Scalar;
    
    return Result;
}


void operator +=(v2f32 &A, v2f32 B)
{
    v2f32 *Result = &A;
    
    Result->x = A.x + B.x;
    Result->y = A.y + B.y;
    
    return;
}


v2f32 v2f32Init(f32 x, f32 y)
{
    v2f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

f32 v2f32Inner(v2f32 A, v2f32 B)
{
    f32 Result = 0.0f;
    
    Result = (A.x * B.x +
              A.y * B.y);
    
    return Result;
}

//- VECTOR 3D 

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


v3f32 operator +(v3f32 A, v3f32 B)
{
    v3f32 Result = { 0 };
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return Result;
}

void operator +=(v3f32 &A, v3f32 B)
{
    v3f32 *Result = &A;
    
    Result->x = A.x + B.x;
    Result->y = A.y + B.y;
    Result->z = A.z + B.z;
    
    return;
}

void operator -=(v3f32 &A, v3f32 B)
{
    v3f32 *Result = &A;
    
    Result->x = A.x - B.x;
    Result->y = A.y - B.y;
    Result->z = A.z - B.z;
    
    return;
}

void operator *=(v3f32 &A, f32 Scalar)
{
    v3f32 *Result = &A;
    
    Result->x = A.x * Scalar;
    Result->y = A.y * Scalar;
    Result->z = A.z * Scalar;
    
    return;
}

b32 operator ==(v3f32 A, v3f32 B)
{
    b32 Result = 0;
    u32 Index  = 0;
    
    while((Result = (A.c[Index] == B.c[Index])) &&
          (Index++ < (3 - 1)));
    
    return Result;
}

f32 v3f32Inner(v3f32 A, v3f32 B)
{
    f32 Result = 0.0f;
    
    Result = (A.x * B.x +
              A.y * B.y +
              A.z * B.z);
    
    return Result;
}

f32 v3f32GetMagnitude(v3f32 A)
{
    f32 Result = 0.0f;
    
    Result = Root(v3f32Inner(A, A));
    
    return Result;
}

v3f32 v3f32Normalize(v3f32 A)
{
    v3f32 Result = { 0.0f };
    
    f32 Magnitude = v3f32GetMagnitude(A);
    
    Result = v3f32Init(A.x / Magnitude,
                       A.y / Magnitude,
                       A.z / Magnitude);
    
    return Result;
}

//- VECTOR 4D 
v4f32 v4f32Init(f32 x, f32 y, f32 z, f32 w)
{
    v4f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

//- RECTANGLE 2D 

b32 rect_v2f32IsOutside(rect_v2f32 Bounds, v2f32 Pos)
{
    b32 Result = 1;
    
    Result = (Bounds.min.x > Pos.x ||
              Bounds.min.y > Pos.y ||
              Bounds.max.x < Pos.x ||
              Bounds.max.y < Pos.y);
    
    return Result;
}

b32 rect_v2f32IsInside(rect_v2f32 Bounds, v2f32 Pos)
{
    b32 Result = 1;
    
    Result = (Bounds.min.x < Pos.x &&
              Bounds.min.y < Pos.y &&
              Bounds.max.x > Pos.x &&
              Bounds.max.y > Pos.y);
    
    return Result;
}

rect_v2f32 rect_v2f32CenteredDim(v2f32 Dim)
{
    rect_v2f32 Result = { 0 };
    
    Result.min.x = -1.0f * Dim.x / 2.0f;
    Result.min.y = -1.0f * Dim.y / 2.0f;
    Result.max.x = Dim.x / 2.0f;
    Result.max.y = Dim.y / 2.0f;
    
    return Result;
}

rect_v2f32 rect_v2f32AddRadiusTo(rect_v2f32 Bounds, v2f32 Dim)
{
    rect_v2f32 Result = Bounds;
    
    v2f32 Radius = Dim * 0.5f;
    
    Result.min.x += -1.0f * Radius.x;
    Result.min.y += -1.0f * Radius.y;
    Result.max.x += Radius.x;
    Result.max.y += Radius.y;
    
    return Result;
}


//- MATRIX 2D 
static void
m2f32Scale(m2f32 *Matrix, f32 ScaleX, f32 ScaleY)
{
    Matrix->x[0][0] = ScaleX;
    Matrix->x[1][1] = ScaleY;
    
    return;
}


//- MATRIX 3D 
#if 0
static void
m2f32Scale(m2f32 *Matrix, f32 ScaleX, f32 ScaleY)
{
    Matrix->x[0][0] = ScaleX;
    Matrix->x[1][1] = ScaleY;
    
    return;
}

static m3f32
m3f32Rotation(f32 x, f32 y)
{
    m4f32 Result = { 0 };
    
    m4f32 RotationX = m4f32Identity();
    if(x != 0.0f)
    {
        RotationX.r[0] = v4f32Init(1.0f,      0.0f, 0.0f);
        RotationX.r[1] = v4f32Init(0.0f, Cosine(x), 0.0f);
        RotationX.r[2] = v4f32Init(0.0f,      0.0f, 1.0f);
    }
    
    m4f32 RotationY = m4f32Identity();
    if(y != 0.0f)
    {
        RotationY.r[0] = v4f32Init(Cosine(y), 0.0f,   Sine(y), 0.0f);
        RotationY.r[1] = v4f32Init(     0.0f, 1.0f,      0.0f, 0.0f);
        RotationY.r[2] = v4f32Init( -Sine(y), 0.0f, Cosine(y), 0.0f);
        RotationY.r[3] = v4f32Init(     0.0f, 0.0f,      0.0f, 1.0f);
    }
    
    
    m4f32 RotationZ = m4f32Identity();
    if(z != 0.0f)
    {
        RotationZ.r[0] = v4f32Init(Cosine(z),  -Sine(z), 0.0f, 0.0f);
        RotationZ.r[1] = v4f32Init(  Sine(z), Cosine(z), 0.0f, 0.0f);
        RotationZ.r[2] = v4f32Init(     0.0f,      0.0f, 1.0f, 0.0f);
        RotationZ.r[3] = v4f32Init(     0.0f,      0.0f, 0.0f, 1.0f);
    }
    
    Result = RotationX * RotationY * RotationZ;
    
    return Result;
}
#endif

//- MATRIX 4D 

m4f32 operator *(m4f32 A, m4f32 B)
{
    m4f32 Result = { 0 };
    
    for(u32 ScanRow = 0; ScanRow < 4; ScanRow++)
    {
        for(u32 ScanCol = 0; ScanCol < 4; ScanCol++)
        {
            f32 *Entry = &Result.r[ScanRow].c[ScanCol];
            
            for(u32 ScanElement = 0; ScanElement < 4; ScanElement++)
            {
                *Entry += A.r[ScanRow].c[ScanElement] * B.r[ScanElement].c[ScanCol];
            }
        }
    }
    
    return Result;
}


b32 operator ==(m4f32 A, m4f32 B)
{
    b32 Result = 0;
    u32 Index  = 0;
    
    while((Result = (A.e[Index] == B.e[Index])) &&
          (Index++ < (16 - 1)));
    
    return Result;
}


static m4f32
m4f32Identity(void)
{
    m4f32 Result = { 0 };
    
    Result.r[0] = v4f32Init(1.0f, 0.0f, 0.0f, 0.0f);
    Result.r[1] = v4f32Init(0.0f, 1.0f, 0.0f, 0.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, 1.0f, 0.0f);
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Result;
}

static m4f32
m4f32Scale(f32 x, f32 y, f32 z)
{
    m4f32 Result = { 0 };
    
    Result.r[0] = v4f32Init(x   , 0.0f, 0.0f, 0.0f);
    Result.r[1] = v4f32Init(0.0f, y   , 0.0f, 0.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, z   , 0.0f);
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Result;
}


static m4f32
m4f32Rotation(f32 x, f32 y, f32 z)
{
    m4f32 Result = { 0 };
    
    m4f32 RotationX = m4f32Identity();
    if(x != 0.0f)
    {
        RotationX.r[0] = v4f32Init(1.0f,      0.0f,      0.0f, 0.0f);
        RotationX.r[1] = v4f32Init(0.0f, Cosine(x),  -Sine(x), 0.0f);
        RotationX.r[2] = v4f32Init(0.0f,   Sine(x), Cosine(x), 0.0f);
        RotationX.r[3] = v4f32Init(0.0f,      0.0f,      0.0f, 1.0f);
    }
    
    m4f32 RotationY = m4f32Identity();
    if(y != 0.0f)
    {
        RotationY.r[0] = v4f32Init(Cosine(y), 0.0f,   Sine(y), 0.0f);
        RotationY.r[1] = v4f32Init(     0.0f, 1.0f,      0.0f, 0.0f);
        RotationY.r[2] = v4f32Init( -Sine(y), 0.0f, Cosine(y), 0.0f);
        RotationY.r[3] = v4f32Init(     0.0f, 0.0f,      0.0f, 1.0f);
    }
    
    
    m4f32 RotationZ = m4f32Identity();
    if(z != 0.0f)
    {
        RotationZ.r[0] = v4f32Init(Cosine(z),  -Sine(z), 0.0f, 0.0f);
        RotationZ.r[1] = v4f32Init(  Sine(z), Cosine(z), 0.0f, 0.0f);
        RotationZ.r[2] = v4f32Init(     0.0f,      0.0f, 1.0f, 0.0f);
        RotationZ.r[3] = v4f32Init(     0.0f,      0.0f, 0.0f, 1.0f);
    }
    
    Result = RotationX * RotationY * RotationZ;
    
    return Result;
}

static m4f32
m4f32Translation(v3f32 PosDelta)
{
    m4f32 Result = { 0 };
#if 0
    Result.r[0] = v4f32Init(1.0f, 0.0f, 0.0f, PosDelta.x);
    Result.r[1] = v4f32Init(0.0f, 1.0f, 0.0f, PosDelta.y);
    Result.r[2] = v4f32Init(0.0f, 0.0f, 1.0f, PosDelta.z);
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
#else
    Result.r[0] = v4f32Init(1.0f, 0.0f, 0.0f, 0.0f);
    Result.r[1] = v4f32Init(0.0f, 1.0f, 0.0f, 0.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, 1.0f, 0.0f);
    Result.r[3] = v4f32Init(PosDelta.x, PosDelta.y, PosDelta.z, 1.0f);
#endif
    return Result;
}

static m4f32
m4f32Viewport(v2f32 WindowDim)
{
    m4f32 Result = { 0 };
    
    Result.r[0] = v4f32Init(WindowDim.x / 2.0f, 0.0f, 0.0f, (WindowDim.x - 1.0f) / 2.0f);
    Result.r[1] = v4f32Init(0.0f, WindowDim.y / 2.0f, 0.0f, (WindowDim.y - 1.0f) / 2.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, 1.0f, 0.0f);
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Result;
}

static m4f32
m4f32Orthographic(f32 LeftPlane,
                  f32 RightPlane,
                  f32 BottomPlane,
                  f32 TopPlane,
                  f32 NearPlane,
                  f32 FarPlane)
{
    m4f32 Result = { 0 };
#if 1
    // NORMALIZING X
    Result.r[0].c[0] = 2.0f / (RightPlane - LeftPlane);
    
    // NORMALIZING Y
    Result.r[1].c[1] = 2.0f / (TopPlane - BottomPlane);
    
    // NORMALIZING Z
    Result.r[2].c[2] = 1.0f / (FarPlane - NearPlane);
    Result.r[2].c[3] = -1.0f * ((NearPlane) / (FarPlane - NearPlane));
    
    // DISREGARDING W
    Result.r[3].c[3] = 1.0f;
#else
    
    // NORMALIZING X
    Result.r[0].c[0] = 2.0f / (RightPlane - LeftPlane);
    Result.r[0].c[3] = -1.0f * ((RightPlane + LeftPlane) / (RightPlane - LeftPlane));
    
    // NORMALIZING Y
    Result.r[1].c[1] = 2.0f / (TopPlane - BottomPlane);
    Result.r[1].c[3] = -1.0f * ((TopPlane + BottomPlane) / (TopPlane - BottomPlane));
    
    // NORMALIZING Z
    Result.r[2].c[2] = 2.0f / (NearPlane - FarPlane);
    Result.r[2].c[3] = -1.0f * ((NearPlane + FarPlane) / (NearPlane - FarPlane));
    
    // DISREGARDING W
    Result.r[3].c[3] = 1.0f;
#endif
    
    return Result;
}

#endif // PHYSICS_SIM_MATH_H
