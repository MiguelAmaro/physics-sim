#include "types.h"
#include "memory.h"
#include "string.h"
#include "os.h"
#include "math.h"
//...
#include "memory.c"
#include "string.c"
#include "win32_core.c"

#include <wchar.h>

#define NULLSTR ((u8 *)0x0ull)
#define TEST_NOPROC ((voidproc *)0x0ull)
#define TEST_NULL  ((void *)0)
#define TEST_TRUE  (1)
#define TEST_FALSE (0)

#define ColorRed     wprintf(L"\x1b[31m")
#define ColorGreen   wprintf(L"\x1b[32m")
#define ColorYellow  wprintf(L"\x1b[33m")
#define ColorBlue    wprintf(L"\x1b[34m")
#define ColorMagenta wprintf(L"\x1b[35m")
#define ColorCyan    wprintf(L"\x1b[96m")
#define ColorReset   wprintf(L"\x1b[0m")

#define PrintAddress(x) wprintf(L"%hs = 0x%llx\n", #x, (u64)(x))
#define Prints32(x) wprintf(L"%hs = %d\n"  , #x, (s32)(x))
#define Prints64(x) wprintf(L"%hs = %lld\n", #x, (s64)(x))
#define Printu32(x) wprintf(L"%hs = %u\n"  , #x, (u32)(x))
#define Printu64(x) wprintf(L"%hs = %llu\n", #x, (u64)(x))
#define Printf32(x) wprintf(L"%hs = %e [%f]\n", #x, (f32)(x), (f32)(x))
#define Printf64(x) wprintf(L"%hs = %e [%f]\n", #x, (f64)(x), (f64)(x))

#define TestDecl(header) \
ColorCyan; \
wprintf(L"RUNNING "#header" UNIT TEST...\n\n"); \
ColorReset

#define TestCond(Condition, Details) \
ColorCyan;   wprintf(L"TEST: %hs\n", "Condition"); \
ColorReset;  wprintf(L"condition: %hs\n", #Condition); \
ColorYellow; wprintf(L"Assertion: "); \
ColorReset;  wprintf(L"%hs\n", Details); \
if(Condition) \
{ \
ColorGreen; \
wprintf(L"PASSED!!!\n"); \
ColorReset; \
} \
else \
{ \
ColorRed; \
wprintf(L"FAILED!!!\n"); \
ColorReset; \
}

#define Test(type, Expected, TestResult, TestName, Assertion) \
{ \
type expect = (Expected); \
type result = (TestResult); \
ColorCyan;   wprintf(L"TEST: %hs\n", TestName); \
ColorReset;  wprintf(L"returns: %hs  expects: %hs\n", #type, #Expected); \
ColorYellow; wprintf(L"Assertion: "); \
ColorReset;  wprintf(L"%hs\n", Assertion); \
if(MemoryIsEqual((u8 *)&expect, (u8 *)&result, sizeof(type))) \
{ \
ColorGreen; \
wprintf(L"PASSED!!!\n"); \
ColorReset; \
} \
else \
{ \
ColorRed; \
wprintf(L"FAILED!!!\n"); \
ColorReset; \
} \
wprintf(L"\n\n"); \
} \

#define TestFunc(type, Function, Expected, ResultDisplay, Assertion) \
{ \
type expect = (Expected); \
type result = (Function); \
ColorCyan;  wprintf(L"TEST: "); \
ColorReset; wprintf(L"%hs\n", #Function); \
if(ResultDisplay) \
{ \
wprintf(L"returns: %hs\n", #type); \
wprintf(L"expects: \n"); \
ResultDisplay(Expected); \
wprintf(L"recieved: \n"); \
ResultDisplay(Function); \
} else \
{ \
wprintf(L"returns: %hs  expects: %hs\n", #type, #Expected); \
} \
ColorYellow; wprintf(L"Assertion: "); \
ColorReset; wprintf(L"%hs\n", Assertion); \
if(MemoryIsEqual(&expect, &result, sizeof(type))) \
{ \
ColorGreen; \
wprintf(L"PASSED!!!\n"); \
ColorReset; \
} \
else \
{ \
ColorRed; \
wprintf(L"FAILED!!!\n"); \
ColorReset; \
} \
wprintf(L"\n\n"); \
} \

typedef void display_m4f(m4f Matrix);

void DisplayM4f(m4f Matrix)
{
  wprintf(L"| %.4f %.4f %.4f %.4f |\n", Matrix.r[0].x, Matrix.r[0].y, Matrix.r[0].z, Matrix.r[0].w);
  wprintf(L"| %.4f %.4f %.4f %.4f |\n", Matrix.r[1].x, Matrix.r[1].y, Matrix.r[1].z, Matrix.r[1].w);
  wprintf(L"| %.4f %.4f %.4f %.4f |\n", Matrix.r[2].x, Matrix.r[2].y, Matrix.r[2].z, Matrix.r[2].w);
  wprintf(L"| %.4f %.4f %.4f %.4f |\n", Matrix.r[3].x, Matrix.r[3].y, Matrix.r[3].z, Matrix.r[3].w);
  return;
}

void DisplayMf4x4Sequentialization(m4f Matrix)
{
  wprintf(L"Matrix Sequentialization\n");
  wprintf(L"| %p %p %p %p |\n", &Matrix.r[0].x, &Matrix.r[0].y, &Matrix.r[0].z, &Matrix.r[0].w);
  wprintf(L"| %p %p %p %p |\n", &Matrix.r[1].x, &Matrix.r[1].y, &Matrix.r[1].z, &Matrix.r[1].w);
  wprintf(L"| %p %p %p %p |\n", &Matrix.r[2].x, &Matrix.r[2].y, &Matrix.r[2].z, &Matrix.r[2].w);
  wprintf(L"| %p %p %p %p |\n", &Matrix.r[3].x, &Matrix.r[3].y, &Matrix.r[3].z, &Matrix.r[3].w);
  return;
}

void DisplayCMf4x4Sequentialization(void)
{
  f32 Matrix[4][4] = {0};
  wprintf(L"C Matrix Sequentialization\n");
  wprintf(L"| %p %p %p %p |\n", &Matrix[0][0], &Matrix[0][1], &Matrix[0][2], &Matrix[0][3]);
  wprintf(L"| %p %p %p %p |\n", &Matrix[1][0], &Matrix[1][1], &Matrix[1][2], &Matrix[1][3]);
  wprintf(L"| %p %p %p %p |\n", &Matrix[2][0], &Matrix[2][1], &Matrix[2][2], &Matrix[2][3]);
  wprintf(L"| %p %p %p %p |\n", &Matrix[3][0], &Matrix[3][1], &Matrix[3][2], &Matrix[3][3]);
  return;
}

typedef struct test_display test_display;
struct test_display
{
  display_m4f *Mat4x4;
};

test_display Display;