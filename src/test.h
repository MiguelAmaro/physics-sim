#include "physics_sim_memory.h"
#include "physics_sim_types.h"
#include "physics_sim_math.h"
#include <wchar.h>

#define TEST_NOPROC (__noop)
#define TEST_NULL  ((void *)0)
#define TEST_TRUE  (1)
#define TEST_FALSE (0)

#define ANSI_COLOR_RED     wprintf(L"\x1b[31m")
#define ANSI_COLOR_GREEN   wprintf(L"\x1b[32m")
#define ANSI_COLOR_YELLOW  wprintf(L"\x1b[33m")
#define ANSI_COLOR_BLUE    wprintf(L"\x1b[34m")
#define ANSI_COLOR_MAGENTA wprintf(L"\x1b[35m")
#define ANSI_COLOR_CYAN    wprintf(L"\x1b[96m")
#define ANSI_COLOR_RESET   wprintf(L"\x1b[0m")

#define Prints32(x) wprintf(L"%hs = %d\n"  , #x, (s32)(x))
#define Prints64(x) wprintf(L"%hs = %lld\n", #x, (s64)(x))
#define Printu32(x) wprintf(L"%hs = %u\n"  , #x, (u32)(x))
#define Printu64(x) wprintf(L"%hs = %llu\n", #x, (u64)(x))
#define Printf32(x) wprintf(L"%hs = %e [%f]\n", #x, (f32)(x), (f32)(x))
#define Printf64(x) wprintf(L"%hs = %e [%f]\n", #x, (f64)(x), (f64)(x))

#define TEST(type, Expected, TestResult, TestName, Assertion) \
{ \
type expect = (Expected); \
type result = (TestResult); \
ANSI_COLOR_CYAN;   wprintf(L"TEST: %hs\n", TestName); \
ANSI_COLOR_RESET;  wprintf(L"returns: %hs  expects: %hs\n", #type, #Expected); \
ANSI_COLOR_YELLOW; wprintf(L"Assertion: "); \
ANSI_COLOR_RESET;  wprintf(L"%hs\n", Assertion); \
if(MemoryIsEqual((u8 *)&expect, (u8 *)&result, sizeof(type))) \
{ \
ANSI_COLOR_GREEN; \
wprintf(L"PASSED!!!\n"); \
ANSI_COLOR_RESET; \
} \
else \
{ \
ANSI_COLOR_RED; \
wprintf(L"FAILED!!!\n"); \
ANSI_COLOR_RESET; \
} \
wprintf(L"\n\n"); \
} \

#define TESTFUNC(type, Function, Expected, ResultDisplay, Assertion) \
{ \
type expect = (Expected); \
type result = (Function); \
ANSI_COLOR_CYAN;  wprintf(L"TEST: "); \
ANSI_COLOR_RESET; wprintf(L"%hs\n", #Function); \
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
ANSI_COLOR_YELLOW; wprintf(L"Assertion: "); \
ANSI_COLOR_RESET; wprintf(L"%hs\n", Assertion); \
if(MemoryIsEqual((u8 *)&expect, (u8 *)&result, sizeof(type))) \
{ \
ANSI_COLOR_GREEN; \
wprintf(L"PASSED!!!\n"); \
ANSI_COLOR_RESET; \
} \
else \
{ \
ANSI_COLOR_RED; \
wprintf(L"FAILED!!!\n"); \
ANSI_COLOR_RESET; \
} \
wprintf(L"\n\n"); \
} \

#define DISPLAYM4F(name) void name(m4f Matrix)
typedef DISPLAYM4F(display_m4f);

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

struct test_display
{
  display_m4f *Mat4x4 = DisplayM4f;
};

test_display Display;