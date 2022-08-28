#include "test.h"

void RunUnitTestsString()
{
  ANSI_COLOR_CYAN;
  wprintf(L"RUNNING STRING UNIT TEST...\n\n");
  ANSI_COLOR_RESET;
  TEST(f32, 10.0f, 11.0f, "Float Test", "Floats in macro test are equal n this works");
  TESTFUNC(b32, Str8IsEqual(Str8(0, 0), Str8(0, 0)),
           TEST_TRUE, TEST_NOPROC,
           "Two empty strings are equal.");
  TESTFUNC(b32, Str8IsEqual(Str8FromCStr("ab"), Str8(0, 0)),
           TEST_FALSE, TEST_NOPROC,
           "An empty string and a string of length one or greater are not equal.");
  TESTFUNC(b32, Str8IsEqual(Str8FromCStr("a"), Str8FromCStr("a")), 
           TEST_TRUE, TEST_NOPROC,
           "Two strings of same length (one) that have the same codepoint are equal.");
  TESTFUNC(b32, Str8IsEqual(Str8FromCStr("abcdefg"), Str8FromCStr("abcdefg")),
           TEST_TRUE, TEST_NOPROC,
           "Two strings of same length (greater than one) that have the same codepoints are equal.");
  TESTFUNC(b32, Str8IsEqual(Str8FromCStr("defgabc"), Str8FromCStr("abcdefg")),
           TEST_FALSE, TEST_NOPROC,
           "Two strings of same length (greater than one) that have different codepoints are not equal.");
  return;
}

b32 RunUnitTestsMath(void)
{
  ANSI_COLOR_CYAN;
  wprintf(L"RUNNING MATH UNIT TEST...\n\n");
  ANSI_COLOR_RESET;
  DisplayMf4x4Sequentialization(M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
                                    V4f(0.0f, 1.0f, 0.0f, 0.0f),
                                    V4f(0.0f, 0.0f, 1.0f, 0.0f),
                                    V4f(0.0f, 0.0f, 0.0f, 1.0f)));
  DisplayCMf4x4Sequentialization();
  TESTFUNC(m4f, M4fOrtho(0.0f, 200.0f, 0.0f, 200.0f, 0.0f, 100.0f),
           M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
               V4f(1.0f, 1.0f, 0.0f, 0.0f),
               V4f(1.0f, 0.0f, 1.0f, 0.0f),
               V4f(1.0f, 0.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "This constucted orthographic matrix constructor should match the given");
  TESTFUNC(m4f, M4fIdentity(),
           M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
               V4f(0.0f, 1.0f, 0.0f, 0.0f),
               V4f(0.0f, 0.0f, 1.0f, 0.0f),
               V4f(0.0f, 0.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "This identity matrix data should layed out in this way.");
  TESTFUNC(m4f, M4fIdentity(),
           M4f(V4f(1.0f, 0.0f, 0.0f, 0.0f),
               V4f(0.0f, 1.0f, 0.0f, 0.0f),
               V4f(0.0f, 0.0f, 1.0f, 0.0f),
               V4f(0.0f, 0.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "This identity matrix data should layed out in this way.");
  TESTFUNC(m4f, M4fMultiply(&M4fIdentity(),
                            &M4f(V4f(1.0f, 2.0f, 0.0f, 1.0f),V4f(1.0f, 2.0f, 0.0f, 1.0f),
                                 V4f(3.0f, 3.0f, 0.0f, 3.0f),V4f(20.0f, 20.0f, 0.0f, 1.0f)),
                            (m4f *)TEST_NULL, 0),
           M4f(V4f(1.0f, 2.0f, 0.0f, 1.0f),
               V4f(1.0f, 2.0f, 0.0f, 1.0f),
               V4f(3.0f, 3.0f, 0.0f, 3.0f),
               V4f(20.0f, 20.0f, 0.0f, 1.0f)), Display.Mat4x4,
           "A matrix, 'A' multiplied by an identity matrix should yield matrix 'A'.");
  
  
  m4f MatRes = M4fMultiply(&M4f(V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(0.0f, 0.0f, 0.0f, 0.0f),
                                V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(0.0f, 0.0f, 0.0f, 0.0f)),
                           &M4f(V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(1.0f, 1.0f, 1.0f, 1.0f),
                                V4f(2.0f, 2.0f, 2.0f, 2.0f),V4f(1.0f, 1.0f, 1.0f, 1.0f)),(m4f *)TEST_NULL, 0);
  TESTFUNC(m4f, MatRes,
           M4f(V4f(16.0f, 8.0f, 16.0f, 8.0f), V4f(0.0f, 0.0f, 0.0f, 0.0f),
               V4f(16.0f, 8.0f, 16.0f, 8.0f), V4f(0.0f, 0.0f, 0.0f, 0.0f)),
           Display.Mat4x4,
           "A matrix, 'A' multiplied by an identity matrix should yield matrix 'A'.");
  Printf64(64);
  return 1;
}

int main()
{
  RunUnitTestsString();
  RunUnitTestsMath();
  return 0;
}