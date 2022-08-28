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


{
  
  //~ UI Core
  void UICoreGetInput()
  {
    
  }
  
  void UICoreComputeAxisPos()
  {
    
    return;
  }
  
  ui_block *UICoreParentStackGetTop(void)
  {
    ui_state *State = GlobalUIState;
    ui_block *TopParent = State->ParentStack + State->ParentCount - 1;
    return TopParent;
  }
  
  void UICoreParentStackPop()
  {
    ui_state *State = GlobalUIState;
    if(State->ParentCount > 0)
    {
      State->ParentCount--;
      ui_block *PoppedParent   = State->ParentStack + State->ParentCount;
      ui_block *RestoredParent = UICoreParentStackGetTop();
      MemorySet(0, PoppedParent, sizeof(ui_block));
      State->ActiveParent = RestoredParent;
    }
    return;
  }
  
  void UICoreParentStackPushBlock(ui_block Parent)
  {
    ui_state *State = GlobalUIState;
    if(State->ParentCount < State->ParentMaxCount)
    {
      ui_block *NewParent = State->ParentStack + State->ParentCount++;
      *NewParent = Parent;
      State->ActiveParent = NewParent;
    }
    return;
  }
  
  void UIStateInit(ui_state *State, render_buffer *RenderBuffer, v2f WindowDim, app_input *Input, arena Arena)
  {
    State->WindowDim = WindowDim;
    State->Input = *Input;
    State->RenderBuffer = RenderBuffer;
    State->ParentCount = 0;
    State->ParentMaxCount = 256;
    State->Arena = Arena;
    State->UIBlockHashCount = 0;
    State->UIBlockHashMaxCount = 1049;
    for(u32 Index=0; Index<State->UIBlockHashMaxCount; Index++)
    {
      ui_block *Block = State->UIBlockHash + Index;
      MemorySet(0, Block, sizeof(ui_block));
      Block->Key.Key = U64MAX;
    }
    GlobalUIState = State;
    return;
  }
  
  void UICoreSetLayoutAxis()
  {
    
  }
  
  ui_block *UICoreLookUpBlock(ui_key Key)
  {
    ui_state *State = GlobalUIState;
    ui_block *Found = SIM_NULL;
    State->UIBlockHash;
    u32 FNVOffsetBasis = 2166136261; // NOTE(MIGUEL): 32Bit offset basis
    u32 FNVPrime       = 16777619;   // NOTE(MIGUEL): 32Bit offset basis
    u32 ByteCount = sizeof(Key.Key);
    Assert(ByteCount == 8);
    u32 DataIndex = 0;
    u8 *Data      = (u8 *)(&Key);
    u32 Hash = FNVOffsetBasis;
    for(u32 ByteIndex=0; ByteIndex<ByteCount; ByteIndex++)
    {
      Hash = Hash*FNVPrime;
      Hash = Hash ^ Data[DataIndex];
    }
    u32 HashIndex = Hash % State->UIBlockHashMaxCount;
    u32 SearchIndex = HashIndex;
    ui_block *Entry = State->UIBlockHash + (SearchIndex % State->UIBlockHashMaxCount);
    if(Entry->Key.Key == Key.Key) Found = Entry;
    while(!Found)
    {
      Entry = State->UIBlockHash + (SearchIndex % State->UIBlockHashMaxCount);
      // NOTE(MIGUEL): Internal Chaining
      if(Entry->Key.Key == U64MAX)
      {
        Found = Entry; break;
      }
      if(SearchIndex++ == HashIndex) break;
    }
    return Found;
  }
  
  /*
  1. Memory for hash table (DONE!!!!)
  2. Make sure hash mem is accesable from createblock(DONE!!!)
  3. Decide on a keying strategyu(go with the easiest) pointer(DONE!!!)
  4. Find some hash function that work with keyinng stategty (DONE!!!)
  5. Test the hash function.
  6. Accept that parent stack is a busted concept and will be ignored for now
  7. Use hash table  for storage of ui widget hierarchy
  8. 
  
  
  */
  
  ui_block *UICoreCreateBlock(const char *Label, u32 Flags, ui_key Key)
  {
    // NOTE(MIGUEL): Maybe base on misunderstanding
    ui_state *State = GlobalUIState;
    ui_block *Parent = UICoreParentStackGetTop();
    ui_block *NewBlock = UICoreLookUpBlock(Key);
    Assert(NewBlock != SIM_NULL);
    NewBlock->Parent      = Parent;
    /*Parent->LastChild->NewBlock->*/
    NewBlock->PrevSibling = Parent->LastChild;
    NewBlock->NextSibling = SIM_NULL;
    NewBlock->FirstChild  = SIM_NULL;
    NewBlock->LastChild   = SIM_NULL;
    if(Parent->FirstChild == SIM_NULL)
    {
      Parent->FirstChild = NewBlock;
    }
    
    //Size
    NewBlock->Size[Axis_X].Option = UI_Size_PercentOfParent;
    NewBlock->Size[Axis_X].Value = 1.0f;
    NewBlock->Size[Axis_X].Strictness = 1.0f;
    
    NewBlock->Size[Axis_Y].Option = UI_Size_PercentOfParent;
    NewBlock->Size[Axis_Y].Value = 0.5f;
    NewBlock->Size[Axis_Y].Strictness = 1.0f;
    
    NewBlock->ComputedSize.x = (Parent->Rect.max.x-Parent->Rect.min.x)*NewBlock->Size[Axis_X].Value;
    NewBlock->ComputedSize.y = (Parent->Rect.max.y-Parent->Rect.min.y)*NewBlock->Size[Axis_Y].Value;
    NewBlock->ComputedRelPos.x  = (Parent->Rect.max.x-Parent->Rect.min.x);
    NewBlock->ComputedRelPos.y  = (Parent->Rect.max.y-Parent->Rect.min.y);
    
    NewBlock->Rect = R2f(0.0f,0.0f,100.0f, 100.0f);
    
    UICoreComputeAxisPos();
    if(Label)
    {
      NewBlock->String = Str8FromCStr(Label);
    }
    return NewBlock;
  }
  
  ui_input GetUIInput(ui_block *Parent)
  {
    ui_state *State = GlobalUIState;
    ui_input Result = {0};
    Result.Hover = R2fIsInside(Parent->Rect, State->Input.MousePos);
    return Result;
  }
  
  //~ End UI Core
  
  //~ UI Builder
  
  ui_input UIBuildButton(const char *Label)
  {
    ui_state *State = GlobalUIState;
    ui_block *Parent = UICoreParentStackGetTop();
    ui_block LastParent = *Parent;
    if(Parent)
    {
      // NOTE(MIGUEL): Flip Coordinates
      r2f Rect = Parent->Rect;
      Rect = R2f(Rect.min.x, State->WindowDim.y-Rect.max.y, Rect.max.x, State->WindowDim.y-Rect.min.y);
      
      DrawRect(GlobalUIState->RenderBuffer, Rect, V3f(1.0f,0.0f,0.0f), Parent->Color);
      DrawSomeText(GlobalUIState->RenderBuffer, Str8FromCStr(Label), 10, V3f(Rect.min.x+10.0f, Rect.min.y+Parent->Size[Axis_Y].Value*0.5f, 1.0f),
                   V4f(1.0f, 1.0f, 1.0f, 1.0f));
      
      // NOTE(MIGUEL): Calculated Position based on sematics
      
      // NOTE(MIGUEL): Final Position
      Parent->Rect= R2f(Parent->Rect.min.x,
                        Parent->Rect.min.y + Parent->Size[Axis_Y].Value,
                        Parent->Rect.max.x,
                        Parent->Rect.max.y + Parent->Size[Axis_Y].Value);
    }
    
    ui_input Result = GetUIInput(&LastParent);
    return Result;
  }
  
  ui_input UIBuildNewButton(const char *Label, ui_key Key)
  {
    ui_state *State = GlobalUIState;
    ui_block *UIBlock = UICoreCreateBlock(Label,
                                          UI_BlockFlag_DrawText |
                                          UI_BlockFlag_HotAnimation |
                                          UI_BlockFlag_DrawBackground, Key);
    UICoreParentStackPushBlock(*UIBlock);
    r2f Rect = UIBlock->Rect;
    Rect = R2f(Rect.min.x, State->WindowDim.y-Rect.max.y, Rect.max.x, State->WindowDim.y-Rect.min.y);
    
    DrawRect(GlobalUIState->RenderBuffer, Rect, V3f(1.0f,0.0f,0.0f), UIBlock->Color);
    DrawSomeText(GlobalUIState->RenderBuffer, Str8FromCStr(Label), 10, V3f(Rect.min.x+10.0f, Rect.min.y+UIBlock->Size[Axis_Y].Value*0.5f, 1.0f),
                 V4f(1.0f, 1.0f, 1.0f, 1.0f));
    
    ui_input Result = GetUIInput(UIBlock);
    return Result;
  }
  
  ui_key UIMakeKey(void *Address, u64 Increment)
  {
    ui_key Key = {0};
    Key.Address = Address;
    Key.Key += Increment;
    Assert(Key.Key != U64MAX);
    return Key;
  }
  
  void UIBuildSomething(const char *Label)
  {
    UICoreCreateBlock(Label,
                      UI_BlockFlag_DrawText |
                      UI_BlockFlag_DrawBackground, MakeKey((void *)69, 0));
    UICoreGetInput();
    return;
  }
  
  void UIBuildBanner()
  {
    ui_state *State = GlobalUIState;
    ui_block *Parent = UICoreParentStackGetTop();
    if(Parent)
    {
      // NOTE(MIGUEL): Flip Coordinates
      r2f Rect = Parent->Rect;
      Rect = R2f(Rect.min.x, State->WindowDim.y-Rect.max.y, Rect.max.x, State->WindowDim.y-Rect.min.y);
      
      DrawRect(GlobalUIState->RenderBuffer, Rect, V3f(1.0f,0.0f,0.0f), Parent->Color);
      Parent->Rect= R2f(Parent->Rect.min.x,
                        Parent->Rect.min.y + Parent->Size[Axis_Y].Value,
                        Parent->Rect.max.x,
                        Parent->Rect.max.y + Parent->Size[Axis_Y].Value);
    }
    return;
  }
  
  void UIBuildBannerList(const char *Label)
  {
    ui_block NewParent = {0};
    ui_block *ActiveParent = SIM_NULL;
    ActiveParent = UICoreParentStackGetTop();
    NewParent.Rect = R2f(ActiveParent->Rect.min.x     , ActiveParent->Rect.min.y,
                         ActiveParent->Rect.max.x*0.5f, ActiveParent->Rect.min.y+4.0f);
    NewParent.Size[Axis_Y].Value = 5.0f;
    NewParent.Color = V4f(0.0f, 1.0f, 1.0f, 1.0f);
    UIBuildBanner();
    
    UICoreParentStackPushBlock(NewParent);
    UIBuildBanner();
    UIBuildBanner();
    UIBuildBanner();
    UIBuildBanner();
    UIBuildBanner();
    UICoreParentStackPop();
    
    return;
  }
  
  
  //~ End UI Builder
  
  /*
  ui components
  Fucntions: Ui element behavior or response to input
  generic struct: ui state (postion, position semantics, )
  
  
  Stack Local layout configureations for ui children (basically this requires a heirachy of layouts)
  First pass: hierarchy is implemented using a stack
  First pass: Layout tracks xy offset; rect corner positions; 
  First pass: offsets are const but rect corner pos are updated for every ui elemnt call 
  
  Ryans Revalations: layout heirarchy should just be ui elm heirarchy
  Second Pass: 
  
  
  Questions:
  What is layout used for? layout is used coneviently position a sequence of ui elms
  What is a layout? layout is data used to coneviently position a sequence of ui elms
  What is a ui widget as ryan defines it? data used to fufile responsabliteis of layout and also features.
  What is ui elm? For me it is any interactable and visable thing on the screen that is created by ui code.
  What is the difference between a funciton that creates/defines a widfet(PushParent) and one that responsd(UI_Button)?
  It seems like ui widet is basically a struct that provede blue print to create a a sequence of ui elm with similar charactics like
  (positionin, size, behavior) and calls like UI_Button(use the last ui widget/blueprint)
  Are we using a stack and binary tree in tandem??
  Why do we need a frame of delay to respond to inputs also why is the order backwareds and why cant we just sort beffore drawing????
  
  whay is a command buffer bad? its very wastefull but wasteful. 
  Is it wastefull to use the command buffer for arbitrary featrues or to render quads?
  */
}