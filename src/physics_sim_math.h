/* date = September 20th 2021 7:46 pm */

#ifndef PHYSICS_SIM_MATH_H
#define PHYSICS_SIM_MATH_H

#include "physics_sim_types.h"
#include "float.h"
#include <immintrin.h>
#include "thirdparty\sse_mathisfun.h"



//-/ MACROS

#define Min(a, b) ((a<b)?a:b)
#define Max(a, b) ((a>b)?a:b)
#define Clamp(x, Low, High) (Min(Max(Low, x), High))

#define KILOBYTES(size) (         (size) * 1024LL)

#define MEGABYTES(size) (KILOBYTES(size) * 1024LL)
#define GIGABYTES(size) (MEGABYTES(size) * 1024LL)
#define TERABYTES(size) (GIGABYTES(size) * 1024LL)

#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))
#define MAXIMUM(a, b) ((a < b) ? (a) : (b))

#define PI32 (3.141592653589793f)
#define GOLDEN_RATIO32 (1.618033988749894f)


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

f32 Absolute(f32 Value)
{
  union
  { 
    f32 f;
    u32 u;
  } Result;
  
  Result.f  = Value;
  Result.u &= 0x7fffffff;
  
  return Result.f;
}

// Series and Sequences for function approximation
// Thomas Calculus pg.637
// Thomas Calculus pg.619
// https://www.ncbi.nlm.nih.gov/pmc/articles/PMC6928950/
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
  f32 Bx_a = Absolute( B * Value);
  f32 Num = Bx_a + Value * Value;
  f32 Atan_1q = Num / ( 1.f + Bx_a + Num );
  
  // Restore the sign Bit
  u32 Atan_2q = ux_s | (u32 &)Atan_1q;
  return (f32 &)Atan_2q;
}

#if 0
f32 ArcTan2(f32 y, f32 x )
{
  f32 Result = 0.0f;
  /*
  const double a = x;
  const double b = y;
  
  __m128d SSEThing = _mm_atan2_pd(_mm_load_sd(&a), _mm_load_sd(&b));
  
  __m128 SSEOtherThing =  _mm_castpd_ps(SSEThing);
  
  _mm_store_ss(&Result, SSEOtherThing);
  */
#if 0
  static const uint32_t sign_mask = 0x80000000;
  static const float b = 0.596227f;
  
  // Extract the sign bits
  uint32_t ux_s  = sign_mask & (uint32_t &)x;
  uint32_t uy_s  = sign_mask & (uint32_t &)y;
  
  // Determine the quadrant offset
  float q = (float)( ( ~ux_s & uy_s ) >> 29 | ux_s >> 30 ); 
  
  // Calculate the arctangent in the first quadrant
  float bxy_a = Absolute( b * x * y );
  float num = bxy_a + y * y;
  float atan_1q =  num / ( x * x + bxy_a + num );
  
  // Translate it to the proper quadrant
  uint32_t uatan_2q = (ux_s ^ uy_s) | (uint32_t &)atan_1q;
  return q + (float &)uatan_2q;
#else
  const float n1 = 0.97239411f;
  const float n2 = -0.19194795f;    
  float result = 0.0f;
  if (x != 0.0f)
  {
    const union { float flVal; u32 nVal; } tYSign = { y };
    const union { float flVal; u32 nVal; } tXSign = { x };
    if (Absolute(x) >= Absolute(y))
    {
      union { float flVal; u32 nVal; } tOffset = { PI32 };
      // Add or subtract PI based on y's sign.
      tOffset.nVal |= tYSign.nVal & 0x80000000u;
      // No offset if x is positive, so multiply by 0 or based on x's sign.
      tOffset.nVal *= tXSign.nVal >> 31;
      result = tOffset.flVal;
      const float z = y / x;
      result += (n1 + n2 * z * z) * z;
    }
    else // Use atan(y/x) = pi/2 - atan(x/y) if |y/x| > 1.
    {
      union { float flVal; u32 nVal; } tOffset = { PI32 / 2.0f };
      // Add or subtract PI/2 based on y's sign.
      tOffset.nVal |= tYSign.nVal & 0x80000000u;            
      result = tOffset.flVal;
      const float z = x / y; 
      result -= (n1 + n2 * z * z) * z;            
    }
  }
  else if (y > 0.0f)
  {
    result = PI32 / 2.0f;
  }
  else if (y < 0.0f)
  {
    result = -PI32 / 2.0f;
  }
#endif
  return Result;
} 
#endif

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

union v2f
{
  struct
  {
    f32 x;
    f32 y;
  };
  f32 c[2];
};

union v2s
{
  struct
  {
    s32 x;
    s32 y;
  };
  s32 c[2];
};

union v3f
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
    v2f xy;
    f32    z;
  };
  f32 c[3];
};

union v4f
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

union r2f
{
  struct
  {
    v2f min;
    v2f max;
  };
  f32 e[2];
};

union r2s
{
  struct
  {
    v2s min;
    v2s max;
  };
  s32 e[4];
};


//- MATRICES 

union m2f
{
  v2f r[2];
  f32   e[4];
  f32   x[2][2];
};

union m3f32
{
  v3f r[3];
  f32   e[9];
  f32   x[3][3];
};


union m4f
{
  v4f r[4];
  f32   e[16];
  f32   x[4][4];
};


//~ OPERATIORS & FUNCTIONS

//- VECTOR 2D 
v2f operator +(v2f A, v2f B)
{
  v2f Result = { 0 };
  
  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  
  return Result;
}

v2f operator +(v2f A, f32 Scalar)
{
  v2f Result = { 0 };
  
  Result.x = A.x + Scalar;
  Result.y = A.y + Scalar;
  
  return Result;
}

v2f operator +(f32 Scalar, v2f A)
{
  v2f Result = A + Scalar;
  
  return Result;
}

v2f operator -(v2f A, v2f B)
{
  v2f Result = { 0 };
  
  Result.x = A.x - B.x;
  Result.y = A.y - B.y;
  
  return Result;
}


v2f operator *(v2f A, f32 Scalar)
{
  v2f Result = { 0 };
  
  Result.x = A.x * Scalar;
  Result.y = A.y * Scalar;
  
  return Result;
}

v2f operator *(f32 Scalar, v2f A)
{
  v2f Result = A * Scalar;
  
  return Result;
}


void operator +=(v2f &A, v2f B)
{
  v2f *Result = &A;
  
  Result->x = A.x + B.x;
  Result->y = A.y + B.y;
  
  return;
}


v2f V2f(f32 x, f32 y)
{
  v2f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  
  return Result;
}

f32 V2fInner(v2f A, v2f B)
{
  f32 Result = 0.0f;
  
  Result = (A.x * B.x +
            A.y * B.y);
  
  return Result;
}

//- VECTOR 3D 

v3f V3f(f32 x, f32 y, f32 z)
{
  v3f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  
  return Result;
}

v3f operator *(v3f A, f32 Scalar)
{
  v3f Result = { 0 };
  
  Result.x = A.x * Scalar;
  Result.y = A.y * Scalar;
  Result.z = A.z * Scalar;
  
  return Result;
}

v3f operator *(f32 Scalar, v3f A)
{
  v3f Result = A * Scalar;
  
  return Result;
}

v3f operator /(v3f A, f32 Scalar)
{
  v3f Result = { 0 };
  
  Result.x = A.x * (1 / Scalar);
  Result.y = A.y * (1 / Scalar);
  Result.z = A.z * (1 / Scalar);
  
  return Result;
}

v3f operator +(v3f A, v3f B)
{
  v3f Result = { 0 };
  
  Result.x = A.x + B.x;
  Result.y = A.y + B.y;
  Result.z = A.z + B.z;
  
  return Result;
}


v3f operator +(v3f A, f32 B)
{
  v3f Result = { 0 };
  
  Result.x = A.x + B;
  Result.y = A.y + B;
  Result.z = A.z + B;
  
  return Result;
}

v3f operator +(f32 B, v3f A)
{
  return A + B;
}

void operator +=(v3f &A, v3f B)
{
  v3f *Result = &A;
  
  Result->x = A.x + B.x;
  Result->y = A.y + B.y;
  Result->z = A.z + B.z;
  
  return;
}

void operator -=(v3f &A, v3f B)
{
  v3f *Result = &A;
  
  Result->x = A.x - B.x;
  Result->y = A.y - B.y;
  Result->z = A.z - B.z;
  
  return;
}

v3f operator -(v3f &A)
{
  v3f Result = { 0 };
  
  Result.x = -A.x;
  Result.y = -A.y;
  Result.z = -A.z;
  
  return Result;
}

v3f operator -(v3f a, v3f b)
{
  v3f Result = { 0 };
  
  Result.x = a.x-b.x;
  Result.y = a.y-b.y;
  Result.z = a.z-b.z;
  
  return Result;
}

void operator *=(v3f &A, f32 Scalar)
{
  v3f *Result = &A;
  
  Result->x = A.x * Scalar;
  Result->y = A.y * Scalar;
  Result->z = A.z * Scalar;
  
  return;
}

b32 operator ==(v3f A, v3f B)
{
  b32 Result = 0;
  u32 Index  = 0;
  
  while((Result = (A.c[Index] == B.c[Index])) &&
        (Index++ < (3 - 1)));
  
  return Result;
}

f32 V3fInner(v3f A, v3f B)
{
  f32 Result = 0.0f;
  
  Result = (A.x * B.x +
            A.y * B.y +
            A.z * B.z);
  
  return Result;
}

f32 V3fLength(v3f A)
{
  f32 Result = 0.0f;
  
  Result = Root(V3fInner(A, A));
  
  return Result;
}

v3f V3fNormalize(v3f a)
{
  v3f Result = { 0.0f };
  f32 Magnitude = V3fLength(a);
  Result = V3f(a.x / Magnitude,
               a.y / Magnitude,
               a.z / Magnitude);
  
  return Result;
}

v3f V3fHadamard(v3f A, v3f B)
{
  v3f Result = {0};
  
  Result.x = A.x * B.x;
  Result.y = A.y * B.y;
  Result.z = A.z * B.z;
  
  return Result;
}

v3f V3fLerp(f32 Bias, v3f a, v3f b)
{
  v3f Result = {0};
  Result.x = a.x*(Bias)+b.x*(1.0f-Bias);
  Result.y = a.y*(Bias)+b.y*(1.0f-Bias);
  Result.z = a.z*(Bias)+b.z*(1.0f-Bias);
  
  return Result;
};


//- VECTOR 4D 
v4f V4f(f32 x, f32 y, f32 z, f32 w)
{
  v4f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  Result.w = w;
  
  return Result;
}

//- RECTANGLE 2D 

b32 R2fIsOutside(r2f Bounds, v2f Pos)
{
  b32 Result = 1;
  
  Result = (Bounds.min.x > Pos.x ||
            Bounds.min.y > Pos.y ||
            Bounds.max.x < Pos.x ||
            Bounds.max.y < Pos.y);
  
  return Result;
}

b32 R2fIsInside(r2f Bounds, v2f Pos)
{
  b32 Result = 1;
  
  Result = (Bounds.min.x <= Pos.x &&
            Bounds.min.y <= Pos.y &&
            Bounds.max.x >= Pos.x &&
            Bounds.max.y >= Pos.y);
  
  return Result;
}

r2f R2fCenteredDim(v2f Dim)
{
  r2f Result = { 0 };
  
  Result.min.x = -1.0f * Dim.x / 2.0f;
  Result.min.y = -1.0f * Dim.y / 2.0f;
  Result.max.x = Dim.x / 2.0f;
  Result.max.y = Dim.y / 2.0f;
  
  return Result;
}

r2f R2fAddRadiusTo(r2f Bounds, v2f Dim)
{
  r2f Result = Bounds;
  
  v2f Radius = Dim * 0.5f;
  
  Result.min.x += -1.0f * Radius.x;
  Result.min.y += -1.0f * Radius.y;
  Result.max.x += Radius.x;
  Result.max.y += Radius.y;
  
  return Result;
}


//- MATRIX 2D 
static void
M2fScale(m2f *Matrix, f32 ScaleX, f32 ScaleY)
{
  Matrix->x[0][0] = ScaleX;
  Matrix->x[1][1] = ScaleY;
  
  return;
}


//- MATRIX 3D 
#if 0
static void
M2fScale(m2f *Matrix, f32 ScaleX, f32 ScaleY)
{
  Matrix->x[0][0] = ScaleX;
  Matrix->x[1][1] = ScaleY;
  
  return;
}

static m3f32
M3f32Rotation(f32 x, f32 y)
{
  m4f Result = { 0 };
  
  M4f RotationX = M4fIdentity();
  if(x != 0.0f)
  {
    RotationX.r[0] = V4f(1.0f,      0.0f, 0.0f);
    RotationX.r[1] = V4f(0.0f, Cosine(x), 0.0f);
    RotationX.r[2] = V4f(0.0f,      0.0f, 1.0f);
  }
  
  M4f RotationY = M4fIdentity();
  if(y != 0.0f)
  {
    RotationY.r[0] = V4f(Cosine(y), 0.0f,   Sine(y), 0.0f);
    RotationY.r[1] = V4f(     0.0f, 1.0f,      0.0f, 0.0f);
    RotationY.r[2] = V4f( -Sine(y), 0.0f, Cosine(y), 0.0f);
    RotationY.r[3] = V4f(     0.0f, 0.0f,      0.0f, 1.0f);
  }
  
  
  M4f RotationZ = M4fIdentity();
  if(z != 0.0f)
  {
    RotationZ.r[0] = V4f(Cosine(z),  -Sine(z), 0.0f, 0.0f);
    RotationZ.r[1] = V4f(  Sine(z), Cosine(z), 0.0f, 0.0f);
    RotationZ.r[2] = V4f(     0.0f,      0.0f, 1.0f, 0.0f);
    RotationZ.r[3] = V4f(     0.0f,      0.0f, 0.0f, 1.0f);
  }
  
  Result = RotationX * RotationY * RotationZ;
  
  return Result;
}
#endif

//- MATRIX 4D 

m4f operator *(m4f A, m4f B)
{
  m4f Result = { 0 };
  
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


b32 operator ==(m4f A, m4f B)
{
  b32 Result = 0;
  u32 Index  = 0;
  
  while((Result = (A.e[Index] == B.e[Index])) &&
        (Index++ < (16 - 1)));
  
  return Result;
}


static m4f
M4fIdentity(void)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}

static m4f
M4fScale(f32 x, f32 y, f32 z)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(x   , 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, y   , 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, z   , 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}

static m4f
M4fRotate2D(f32 cos, f32 sin)
{
  m4f Result = M4fIdentity();
  
  Result.r[0] = V4f( cos, -sin, 0.0f, 0.0f);
  Result.r[1] = V4f( sin,  cos, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}

static m4f
M4fRotate(f32 x, f32 y, f32 z)
{
  m4f Result = { 0 };
  
  m4f RotationX = M4fIdentity();
  if(x != 0.0f)
  {
    RotationX.r[0] = V4f(1.0f,      0.0f,      0.0f, 0.0f);
    RotationX.r[1] = V4f(0.0f, Cosine(x),  -Sine(x), 0.0f);
    RotationX.r[2] = V4f(0.0f,   Sine(x), Cosine(x), 0.0f);
    RotationX.r[3] = V4f(0.0f,      0.0f,      0.0f, 1.0f);
  }
  
  m4f RotationY = M4fIdentity();
  if(y != 0.0f)
  {
    RotationY.r[0] = V4f(Cosine(y), 0.0f,   Sine(y), 0.0f);
    RotationY.r[1] = V4f(     0.0f, 1.0f,      0.0f, 0.0f);
    RotationY.r[2] = V4f( -Sine(y), 0.0f, Cosine(y), 0.0f);
    RotationY.r[3] = V4f(     0.0f, 0.0f,      0.0f, 1.0f);
  }
  
  
  m4f RotationZ = M4fIdentity();
  if(z != 0.0f)
  {
    RotationZ.r[0] = V4f(Cosine(z),  -Sine(z), 0.0f, 0.0f);
    RotationZ.r[1] = V4f(  Sine(z), Cosine(z), 0.0f, 0.0f);
    RotationZ.r[2] = V4f(     0.0f,      0.0f, 1.0f, 0.0f);
    RotationZ.r[3] = V4f(     0.0f,      0.0f, 0.0f, 1.0f);
  }
  
  Result = RotationX * RotationY * RotationZ;
  
  return Result;
}

static m4f
M4fTranslate(v3f PosDelta)
{
  m4f Result = { 0 };
#if 1
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, PosDelta.x);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, PosDelta.y);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, PosDelta.z);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
#else
  Result.r[0] = V4f(1.0f, 0.0f, 0.0f, 0.0f);
  Result.r[1] = V4f(0.0f, 1.0f, 0.0f, 0.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(PosDelta.x, PosDelta.y, PosDelta.z, 1.0f);
#endif
  return Result;
}

static m4f
M4fViewport(v2f WindowDim)
{
  m4f Result = { 0 };
  
  Result.r[0] = V4f(WindowDim.x / 2.0f, 0.0f, 0.0f, (WindowDim.x - 1.0f) / 2.0f);
  Result.r[1] = V4f(0.0f, WindowDim.y / 2.0f, 0.0f, (WindowDim.y - 1.0f) / 2.0f);
  Result.r[2] = V4f(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = V4f(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}

static m4f
M4fOrtho(f32 LeftPlane,
         f32 RightPlane,
         f32 BottomPlane,
         f32 TopPlane,
         f32 NearPlane,
         f32 FarPlane)
{
  m4f Result = { 0 };
#if 0
  // NOTE(MIGUEL): This path has the origin in the middle of the viewport
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
  // NOTE(MIGUEL): This path has the origin in the lowerleft corner of the viewport
  
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
