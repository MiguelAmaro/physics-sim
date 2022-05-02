#include "types.h"
#include "physics_sim_math.h"

#define Prints32(x) printf("%s = %d\n"  , #x, (s32)(x))
#define Prints64(x) printf("%s = %lld\n", #x, (s64)(x))
#define Printu32(x) printf("%s = %u\n"  , #x, (u32)(x))
#define Printu64(x) printf("%s = %llu\n", #x, (u64)(x))
#define Printf32(x) printf("%s = %e [%f]\n", #x, (f32)(x), (f32)(x))
#define Printf64(x) printf("%s = %e [%f]\n", #x, (f64)(x), (f64)(x))

b32 TestMath(void)
{
  
  Printf64(64);
#if 0
  b32 Success = 0;
  
  char  OutputBuffer[1024];
  
  wsprintf(OutputBuffer, "RUNNING MATH FUNCTION TESTS: \n");
  //OutputDebugStringA(OutputBuffer);
  
  MemorySetTo(0, OutputBuffer, sizeof(OutputBuffer));
  
  // TODO(MIGUEL): TEST SIN
  // TODO(MIGUEL): TEST COS
  
  f32 TestResult  = ArcTan(1.0f / 1.0f);
  s32 PrintTestResult     = TestResult * 100;
  s32 PrintExpectedResult = 0.785398f * 100;
  Success = (TestResult == 0.785398f);
  
  wsprintf(OutputBuffer, "Test [Arctan(PI32 / 2.0f)] [%s]"
           " RESULT: %d.2 | EXECTED: %d.2",
           Success? "SUCCESS!": "FAILED!",
           PrintTestResult,
           PrintExpectedResult);
  OutputDebugStringA(OutputBuffer);
  MemorySetTo(0, OutputBuffer, sizeof(OutputBuffer));
  
  
  render_entry  RenderEntry;
  RenderEntry.Type  = RenderType_quad;
  RenderEntry.Pos   = GlyphQuadPos;
  RenderEntry.Dim   = GlyphQuadDim;
  // TODO(MIGUEL): Add some checking
  u8 *RenderData = RenderEntry.Data;
  size_t RenderDataSize = RENDER_ENTRY_DATASEG_SIZE;
  // NOTE(MIGUEL): !!!CRITICAL!!!!This is very sensitive code. The order in which you 
  //               pop elements matters. Changing order will produce garbage data.
  RenderCmdPushDataElm(&RenderData, &RenderDataSize, &CosSin, sizeof(CosSin));
  ASSERT((RenderData-RenderEntry.Data)==sizeof(CosSin));
  RenderCmdPushDataElm(&RenderData, &RenderDataSize, &IsText, sizeof(IsText));
  ASSERT((RenderData-RenderEntry.Data)==(sizeof(CosSin)+sizeof(IsText)));
  RenderCmdPushDataElm(&RenderData, &RenderDataSize, &BitmapData, sizeof(BitmapData));
  ASSERT((RenderData-RenderEntry.Data)==(sizeof(CosSin)+sizeof(IsText))+sizeof(BitmapData) );
  RenderData = (u8 *)&RenderEntry.Data;
  v3f *PtrCosSin = (v3f *)(RenderData + 0);
  b32 *PtrIsText = (b32 *)(RenderData + sizeof(v3f));
  bitmapdata *PtrBitmapData = (bitmapdata *)(RenderData + sizeof(v3f) + sizeof(b32));
  ASSERT(PtrCosSin->x == CosSin.x &&
         PtrCosSin->y == CosSin.y &&
         PtrCosSin->z == CosSin.z);
  ASSERT(*PtrIsText == IsText);
  ASSERT(PtrBitmapData->Width == BitmapData.Width &&
         PtrBitmapData->Height == BitmapData.Height &&
         PtrBitmapData->Pixels == BitmapData.Pixels &&
         PtrBitmapData->BytesPerPixel == BitmapData.BytesPerPixel);
  //ArcTan2();
#endif
  
  return Success;
}
