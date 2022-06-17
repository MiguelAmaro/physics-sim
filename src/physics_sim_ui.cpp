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

void UICoreStateInit(ui_state *State, render_buffer *RenderBuffer, v2f WindowDim, app_input *Input, memory_arena Arena)
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
  
  // NOTE(MIGUEL): This maybe a mis underastatnding of ryans ui guide
#if 0
  ui_block Window = {0};
  Window.FirstChild = SIM_NULL;
  Window.LastChild = SIM_NULL;
  Window.PrevSibling = SIM_NULL;
  Window.NextSibling = SIM_NULL;
  //Size
  Window.Size[Axis_X].Option = UI_Size_Pixels;
  Window.Size[Axis_X].Value = WindowDim.x;
  Window.Size[Axis_X].Strictness = 1.0f;
  
  Window.Size[Axis_Y].Option = UI_Size_Pixels;
  Window.Size[Axis_Y].Value = WindowDim.y;
  Window.Size[Axis_Y].Strictness = 1.0f;
  
  //Position
  ui_block *Parent = UICoreParentStackGetTop();
  if(Parent)
  {
#if 0
    // NOTE(MIGUEL): This path is here to imagin how to build layout on first run.
    Window->ComputedRelPos.x;
    Window->ComputedSize;
    Window->Rect;
#endif
  }
  else
  {
    Window.Rect = R2f(0.0f,0.0f,WindowDim.x, WindowDim.y);
  }
  UICoreBlockPush(Window);
#endif
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

ui_key MakeKey(void *Address, u64 Increment)
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