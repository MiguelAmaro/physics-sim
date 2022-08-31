#include "test.h"

void RunUnitTestsString()
{
  TestDecl(STRING);
  TestFunc(b32, Str8IsEqual(Str8(NULLSTR, 0), Str8(NULLSTR, 0)),
           TEST_TRUE, TEST_NOPROC,
           "Two empty strings are equal.");
  TestFunc(b32, Str8IsEqual(Str8FromCStr("ab"), Str8(NULLSTR, 0)),
           TEST_FALSE, TEST_NOPROC,
           "An empty string and a string of length one or greater are not equal.");
  TestFunc(b32, Str8IsEqual(Str8FromCStr("a"), Str8FromCStr("a")), 
           TEST_TRUE, TEST_NOPROC,
           "Two strings of same length (one) that have the same codepoint are equal.");
  TestFunc(b32, Str8IsEqual(Str8FromCStr("abcdefg"), Str8FromCStr("abcdefg")),
           TEST_TRUE, TEST_NOPROC,
           "Two strings of same length (greater than one) that have the same codepoints are equal.");
  TestFunc(b32, Str8IsEqual(Str8FromCStr("defgabc"), Str8FromCStr("abcdefg")),
           TEST_FALSE, TEST_NOPROC,
           "Two strings of same length (greater than one) that have different codepoints are not equal.");
  return;
}

b32 RunUnitTestsMath(void)
{
  TestDecl(MATH);
  Test(f32, 10.0f, 11.0f, "Float Test", "Floats in macro test are equal n this works");
  DisplayMf4x4Sequentialization(M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
                                    V4f(0.0f, 1.0f, 0.0f, 0.0f),
                                    V4f(0.0f, 0.0f, 1.0f, 0.0f),
                                    V4f(0.0f, 0.0f, 0.0f, 1.0f)));
  DisplayCMf4x4Sequentialization();
  TestFunc(m4f, Orthom4f(0.0f, 200.0f, 0.0f, 200.0f, 0.0f, 100.0f),
           M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
               V4f(1.0f, 1.0f, 0.0f, 0.0f),
               V4f(1.0f, 0.0f, 1.0f, 0.0f),
               V4f(1.0f, 0.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "This constucted orthographic matrix constructor should match the given");
  TestFunc(m4f, Identitym4f(),
           M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
               V4f(0.0f, 1.0f, 0.0f, 0.0f),
               V4f(0.0f, 0.0f, 1.0f, 0.0f),
               V4f(0.0f, 0.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "This identity matrix data should layed out in this way.");
  TestFunc(m4f, Identitym4f(),
           M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
               V4f(0.0f, 1.0f, 0.0f, 0.0f),
               V4f(0.0f, 0.0f, 1.0f, 0.0f),
               V4f(0.0f, 0.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "This identity matrix data should layed out in this way.");
  TestFunc(m4f, 
           Multm4f(Identitym4f(), M4f(V4f(1.0f, 2.0f, 0.0f, 1.0f),V4f(1.0f, 2.0f, 0.0f, 1.0f),
                                      V4f(3.0f, 3.0f, 0.0f, 3.0f),V4f(20.0f, 20.0f, 0.0f, 1.0f))),
           M4f(V4f(1.0f, 2.0f, 0.0f, 1.0f),
               V4f(1.0f, 2.0f, 0.0f, 1.0f),
               V4f(3.0f, 3.0f, 0.0f, 3.0f),
               V4f(20.0f, 20.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "A matrix, 'A' multiplied by an identity matrix should yield matrix 'A'.");
  
  
  m4f MatRes = Multm4f(M4f(V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(0.0f, 0.0f, 0.0f, 0.0f),
                           V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(0.0f, 0.0f, 0.0f, 0.0f)),
                       M4f(V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(1.0f, 1.0f, 1.0f, 1.0f),
                           V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(1.0f, 1.0f, 1.0f, 1.0f)));
  TestFunc(m4f, MatRes,
           M4f(V4f(16.0f, 8.0f, 16.0f, 8.0f), V4f(0.0f, 0.0f, 0.0f, 0.0f),
               V4f(16.0f, 8.0f, 16.0f, 8.0f), V4f(0.0f, 0.0f, 0.0f, 0.0f)),
           Display.Mat4x4,
           "A matrix, 'A' multiplied by an identity matrix should yield matrix 'A'.");
  Printf64(64);
  return 1;
}
void RunUnitTestsOS(void)
{
  TestDecl("OS");
  os_state *State;
  OSStateInit(&State);
  os_thread_ctx ThreadCtx;
  
  /*
RESERVATIONS:
I allocate some memory for a pools in osstate to be used by diffent thread ctx. each threadctx as writen now has a pool
of arenas. i basically give a that memory to to the main thread for it's ctx's pool of arenas. I dont think other thread
will have memory for their pools.
ThreadCtxInit as is written now takes a backbuffer for its pool of arenas. it assigns a pointer of each arena in the pool
but that point is just the same pointer to the back buffer. This is essentially is creating arenas that will overwrite each
 other.
Need convenient way to use base memory functions 
*/
  OSThreadCtxInit(&ThreadCtx, State->Pool, State->PoolSize);
  OSThreadCtxSet(&ThreadCtx);
  arena *Scratch1 = OSThreadCtxGetScratch(&ThreadCtx, NULL, 0); //First scratch no need to test
  u32 *nums = ArenaPushArray(Scratch1, 100, u32);
  arena *Scratch2 = OSThreadCtxGetScratch(&ThreadCtx, &Scratch1, 1); //First scratch no need to test
  PrintAddress(Scratch1);
  PrintAddress(Scratch2);
  return;
}
void RunUnitTestsBase(void)
{
  v3f A = V3f(1.0, 0.0, 0.0);
  v3f B = V3f(1.0, 0.0, 1.0);
  v3f C = V3f(1.0, 0.0, 0.0);
  TestCond(MemoryIsEqual(&A, &B, sizeof(v3f)) == TEST_TRUE, "vectors A and B are not equal");
  TestCond(MemoryIsEqual(&A, &C, sizeof(v3f)) == TEST_TRUE, "vectors A and C are equal");
  return;
}
int main()
{
  Display.Mat4x4 = DisplayM4f;
  RunUnitTestsBase();
  RunUnitTestsOS();
  RunUnitTestsString();
  RunUnitTestsMath();
  return 0;
}