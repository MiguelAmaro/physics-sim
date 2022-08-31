#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <float.h>

#define  SIM_NULL  (0)
#define  SIM_TRUE  (1)
#define  SIM_FALSE (0)

// TAGS
#define fn
#define global

// LANG MACRO EXTENTIONS
#define Ref(type) type *
#define APIPROC __declspec(dllexport)
#define ARG1(a, ...) (a)
#define foreach(a, b) for(s64 a=0;a<b;a++)
#define iterate(type, current, list) for(type *current = list->Next; \
current != NULL; \
current = current->Next)

// ASSERTIONS
#define Assert(expression) if(!(expression)){ __debugbreak(); }
#define CTASTR2(pre,post) pre ## post
#define CTASTR(pre,post) CTASTR2(pre,post)
#define StaticAssert(cond,msg) \
typedef struct { int CTASTR(static_assertion_failed_,msg) : !!(cond); } \
CTASTR(static_assertion_failed_,__COUNTER__)

// UTIL MACROS
#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))
#define Min(a, b) ((a<b)?a:b)
#define Max(a, b) ((a>b)?a:b)
#define Clamp(x, Low, High) (Min(Max(Low, x), High))

#define Kilobytes(size) (         (size) * 1024LL)
#define Megabytes(size) (Kilobytes(size) * 1024LL)
#define Gigabytes(size) (Megabytes(size) * 1024LL)
#define Terabytes(size) (Gigabytes(size) * 1024LL)

// MATHEMATICAL CONSTS
#define Pi32     (3.141592653589793f)
#define Golden32 (1.618033988749894f)

typedef size_t memory_index;
typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef  int8_t  s8 ;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;
typedef  uint8_t  b8 ;
typedef  uint16_t b16;
typedef  uint32_t b32;
typedef  uint64_t b64;
typedef float f32;
typedef double f64;
typedef void voidproc(void);

#define F32MAX FLT_MAX
#define U32MAX UINT32_MAX
#define U64MAX UINT64_MAX
#define S32MAX INT32_MAX
#define S64MAX INT64_MAX


//TIME
typedef struct datetime datetime;
struct datetime
{ s16 year; u8 mon; u8 day; u8 hour; u8 min; u8 sec; u16 ms; };

//VECTORS
typedef union v2f v2f;
union v2f
{
  struct { f32 x; f32 y; };
  f32 e[2];
};
typedef union v2s v2s;
union v2s
{
  struct { s32 x; s32 y; };
  s32 e[2];
};
typedef union v3f v3f;
union v3f
{
  struct { f32 x; f32 y; f32 z; };
  struct { f32 r; f32 g; f32 b; };
  struct { v2f xy; f32 _ignored00; };
  struct { f32 _ignored01; v2f yz; };
  f32 e[3];
};
typedef union v4f v4f;
union v4f
{
  struct { f32 x; f32 y; f32 z; f32 w; };
  struct { f32 r; f32 g; f32 b; f32 a; };
  f32 e[4];
};
typedef union v4u v4u;
union v4u
{
  struct { u32 x; u32 y; u32 z; u32 w; };
  struct { u32 r; u32 g; u32 b; u32 a; };
  u32 e[4];
};
// Inteivals
typedef union i2f i2f;
union i2f
{
  struct { f32 minx; f32 miny; f32 maxx; f32 maxy; };
  struct { v2f min ; v2f max ; };
  f32 e[2];
};
typedef union i2s64 i2s64;
union i2s64
{
  struct { s64 minx; s64 miny; s64 max; s64 maxy; };
  s32 e[4];
};
// MATRICES
typedef union m2f m2f;
union m2f
{
  v2f r[2]; f32 e[4]; f32 x[2][2];
};
typedef union m3f m3f;
union m3f
{
  v3f r[3]; f32 e[9]; f32 x[3][3];
};
typedef union m4f m4f;
union m4f
{
  v4f r[4]; f32 e[16]; f32 x[4][4];
};
//- INITIALIZERS
fn v2f V2f(f32 x, f32 y)
{
  v2f Result = { x, y };
  return Result;
}
fn v2s V2s(s32 x, s32 y)
{
  v2s Result = { x, y };
  return Result;
}
fn v3f V3f(f32 x, f32 y, f32 z)
{
  v3f Result = { x, y, z };
  return Result;
}
fn v4f V4f(f32 x, f32 y, f32 z, f32 w)
{
  v4f Result = { x, y, z, w };
  return Result;
}
fn v4u V4u(u32 x, u32 y, u32 z, u32 w)
{
  v4u Result = { x, y, z, w };
  return Result;
}
fn i2f I2f(f32 minx, f32 miny, f32 maxx, f32 maxy)
{
  Assert(minx<=maxx); Assert(miny<=maxy);
  i2f Result = { minx, miny, maxx, maxy };
  return Result;
}
fn m4f M4f(v4f r0, v4f r1, v4f r2, v4f r3)
{
  m4f Result = { r0, r1, r2, r3 };
  return Result;
}
fn b32 IsPowerOfTwo(u64 a)
{
  b32 Result = (a & (a-1)) == 0;
  return Result;
}
fn u32 SafeTruncateu64(u64 Value)
{
  Assert(Value <= 0xffffffff);
  u32 Result = (u32)Value;
  return Result;
}
typedef struct bit_scan_result bit_scan_result;
struct bit_scan_result
{
  b32 Found;
  u32 Index;
};
fn bit_scan_result FindLeastSignificantSetBit(u32 Value)
{
  bit_scan_result Result = {0};
#if COMPILER_MSVC
  Result.Found = _BitScanForward(&Result.Index, Value);
#else
  for(u32 Test = 0; Test < 32; ++Test)
  {
    if(Value & (1 << Test))
    { Result.Index = Test; Result.Found = 1; break; }
  }
#endif
  return Result;
}

#endif //TYPES_H
