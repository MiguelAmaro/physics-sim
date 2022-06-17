// NOTE(MIGUEL): Old ui code from readin ryans articles
{
  // NOTE(MIGUEL): Layout is esentially ui_box
  struct ui_layout
  {
    //f32 h = 50;
    //f32 yhigh = AppState->WindowDim.y;
    //f32 ylow = yhigh - h; 
    v2f Offset;
    //spacing???
    v2f LastPos;
    r2f Rect;
    v4f Color;
  };
  
#if 0
  void UIButton(u8 *Label)
  {
    // Click Response
    
    //...
    
    // EndClick Response
    render_buffer *RenderBuffer = GlobalUIState.RenderBuffer;
    r2f Rect = ActiveUILayout->Rect;
    v4f Color = ActiveUILayout->Color;
    v3f EulerAngles = V3f(1.0, 0.0, 0.0);
    v3f Dim = V3f(Rect.max.x - Rect.min.x, Rect.max.y - Rect.min.y, 0.0f);
    v3f Pos = V3f(Rect.min.x + Dim.x*0.5f, Rect.min.y + Dim.y*0.5f, 0.5f);
    RenderCmdPushQuad(RenderBuffer, Pos, Dim, EulerAngles, 1, Color);
    str8 Text = Str8FromCStr(Label);
    DrawSomeText(RenderBuffer, Text, 0, V3f(Rect.min.x, Pos.y, 0.0f), V4f(1.0,1.0,0.0,1.0));
    //Set Layout state for next
    ActiveUILayout->Rect.min.y -= ActiveUILayout->Offset.y;
    ActiveUILayout->Rect.max.y -= ActiveUILayout->Offset.y;
    return;
  }
#endif
  
  
  void UIButton(render_buffer *RenderBuffer, ui_layout *Layout, u8 *Label)
  {
    // Click Response
    
    //...
    
    // EndClick Response
    
    r2f Rect = Layout->Rect;
    v3f EulerAngles = V3f(1.0, 0.0, 0.0);
    v3f Dim = V3f(Rect.max.x - Rect.min.x, Rect.max.y - Rect.min.y, 0.0f);
    v3f Pos = V3f(Rect.min.x + Dim.x*0.5f, Rect.min.y + Dim.y*0.5f, 0.5f);
    RenderCmdPushQuad(RenderBuffer, Pos, Dim, EulerAngles, 1, Layout->Color);
    str8 Text = Str8FromCStr(Label);
    DrawSomeText(RenderBuffer, Text, 0, V3f(Rect.min.x, Pos.y, 0.0f), V4f(1.0,1.0,0.0,1.0));
    //Set Layout state for next
    Layout->Rect.min.y -= Layout->Offset.y;
    Layout->Rect.max.y -= Layout->Offset.y;
    return;
  }
  
  
  void UIButton(render_buffer *RenderBuffer, u8 *Label, r2f Rect)
  {
    // Click Response
    
    //...
    
    // EndClick Response
    
    //r2f Rect = R2f(10.0f, 100.0f, 200.0f, 150.0f);
    v3f EulerAngles = V3f(1.0, 0.0, 0.0);
    v3f Dim = V3f(Rect.max.x - Rect.min.x, Rect.max.y - Rect.min.y, 0.0f);
    v3f Pos = V3f(Rect.min.x + Dim.x*0.5f, Rect.min.y + Dim.y*0.5f, 0.5f);
    RenderCmdPushQuad(RenderBuffer, Pos, Dim, EulerAngles, 1, V4f(0.7f,0.7f,.7f,1.0f));
    str8 Text = Str8FromCStr(Label);
    DrawSomeText(RenderBuffer, Text, 0, V3f(Rect.min.x, Pos.y, 0.0f), V4f(1.0f,0.0f,1.0f,1.0f));
    return;
  }
}

{
  // NOTE(MIGUEL): Usage code for old UI Core test
  DrawRect(RenderBuffer, R2f(-2.0f, -2.0f, 2.0f, 2.0f), V3f(1.0f,0.0f,0.0f), V4f(1.0f,0.0f,0.0f,1.0f));
  f32 h = 50;
  f32 w = 200;
  f32 left = 10.0f;
  f32 yhigh = AppState->WindowDim.y;
  f32 ylow = yhigh - h; 
  UIButton(RenderBuffer, (u8 *)"My Button", R2f(left, ylow, left+w, yhigh));
  ylow -= h; 
  yhigh -= h; 
  UIButton(RenderBuffer, (u8 *)"My New Button", R2f(left, ylow, left+w, yhigh));
  yhigh -= h; 
  ylow -= h; 
  UIButton(RenderBuffer, (u8 *)"My Third Button", R2f(left, ylow, left+w, yhigh));
  
  w = 300.0;
  left = 400.0;
  yhigh = AppState->WindowDim.y;
  ylow = yhigh - h; 
  ui_layout UILayout = UILayoutInit(R2f(left, ylow, left+w, yhigh), V2f(0.0, h), V4f(0.3,0.0,0.4,1.0 ));
  UIButton(RenderBuffer, &UILayout, (u8 *)"Easy button");
  UIButton(RenderBuffer, &UILayout, (u8 *)"Easy button 0");
  UIButton(RenderBuffer, &UILayout, (u8 *)"Easy button 1");
  UIButton(RenderBuffer, &UILayout, (u8 *)"Easy button 2");
  w = 40.0;
  left = 0.0;
  yhigh = AppState->WindowDim.y - 200.0f;
  ylow = yhigh - h; 
  UIStateInit(RenderBuffer);
  ui_layout NewUILayout = UILayoutInit(R2f(left, ylow, left+w, yhigh), V2f(0.0, h), V4f(0.6,1.0,0.8,1.0));
  UILayoutSelect(&NewUILayout);
  UIButton((u8 *)"A");
  UIButton((u8 *)"B");
  UIButton((u8 *)"C");
  UIButton((u8 *)"D");
  UIButton((u8 *)"3");
  w = 200.0;
  h = 200.0;
  left = AppState->WindowDim.x/2.0f;
  yhigh = AppState->WindowDim.y/2.0f;
  ylow = yhigh - h; 
  ui_layout StackLayout = UILayoutInit(R2f(left, ylow, left+w, yhigh), V2f(0.0, h), V4f(0.6,1.0,0.8,1.0));
  UILayoutPush(StackLayout);
  UIButton((u8 *)"A");
  ui_layout StackSubLayout = UILayoutInit(R2f(0.0f, 0.0f, 10.f, 10.f), V2f(0.0, 20.), V4f(1.0,0.0,0.0,1.0));
  UILayoutPush(StackSubLayout);
  UIButton((u8 *)"B");
  UIButton((u8 *)"c");
  
}
{
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
}


#include <wchar.h>
void ViewM128(const char *Label, __m128 a)
{
  v4f Vector = {0};
  _mm_store_ps (Vector.c, a);
  wprintf(L"m128: [ %.4f %.4f %.4f %.4f ] : %hs\n",
          Vector.x, Vector.y, Vector.z, Vector.w,
          Label);
  return;
}
void ViewData(void)
{
#if 0
  ViewM128("Wierd Matrix Row[0]", bc0);
  wprintf(L"(Times)\n");
  ViewM128("Id", ar0);
  ViewM128("Id", ar1);
  ViewM128("Id", ar2);
  ViewM128("Id", ar3);
  wprintf(L"(Equals)\n");
  ViewM128("WM*Id", prod0);
  ViewM128("WM*Id", prod1);
  ViewM128("WM*Id", prod2);
  ViewM128("WM*Id", prod3);
  wprintf(L"(Transposed)\n");
  ViewM128("WM*Id", res0);
  ViewM128("WM*Id", res1);
  ViewM128("WM*Id", res2);
  ViewM128("WM*Id", res3);
  wprintf(L"\n\n");
  //tests transpose
  ViewM128("B columns", bc0);
  ViewM128("B columns", bc1);
  ViewM128("B columns", bc2);
  ViewM128("B columns", bc3);
  wprintf(L"(Transposed)\n");
  _MM_TRANSPOSE4_PS(bc0, bc1, bc2, bc3);
  ViewM128("B columns", bc0);
  ViewM128("B columns", bc1);
  ViewM128("B columns", bc2);
  ViewM128("B columns", bc3);
  
#endif
  return;
}
