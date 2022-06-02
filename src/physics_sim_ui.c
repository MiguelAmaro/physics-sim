//~ UI Core
void GetUIInput()
{
  
}

void UIBoxCreate(u8 *Label, u32 Flags)
{
  ui_state *State = &GlobalUIState;
  //Allocate
  ui_block *UIBlock = ArenaPushStruct(&State->Arena, ui_block);
  
  
  return;
}

ui_block UIBlockInit(r2f Rect, v2f Offset, v4f Color)
{
  ui_block Result = {0};
  //Result.Color = Color;
  //Result.Rect = Rect;
  //Result.Offset = Offset;
  return Result;
}

ui_block *UIParentStackGetTop(void)
{
  ui_state *State = &GlobalUIState;
  ui_block *TopParent = State->ParentStack + State->ParentCount - 1;
  return TopParent;
}

void UIParentStackPop()
{
  ui_state *State = &GlobalUIState;
  if(State->ParentCount > 0)
  {
    State->ParentCount--;
    ui_block *PoppedParent   = State->ParentStack + State->ParentCount;
    ui_block *RestoredParent = UIParentStackGetTop();
    MemorySet(0, PoppedParent, sizeof(ui_block));
    State->ActiveParent = RestoredParent;
  }
  return;
}

void UIBlockPush(ui_block Parent)
{
  ui_state *State = &GlobalUIState;
  if(State->ParentCount < State->ParentMaxCount)
  {
    if(State->ParentCount>0)
    {
      // TODO(MIGUEL): Auto layout algo???
      //??????
    }
    ui_block *NewParent = State->ParentStack + State->ParentCount++;
    *NewParent = Parent;
    
    State->ActiveParent = NewParent;
  }
  return;
}

void UIStateInit(render_buffer *RenderBuffer)
{
  ui_state Result = {0};
  Result.RenderBuffer = RenderBuffer;
  Result.ParentCount = 0;
  Result.ParentMaxCount = 256;
  GlobalUIState = Result;
  return;
}

//~ End UI Core

//~ UI Builder

void UIButton(u8 *Label)
{
  UIBoxCreate(Label,
              UI_WidgetFlag_DrawText |
              UI_WidgetFlag_DrawBackground);
  GetUIInput();
}

void UIBu(u8 *Label)
{
  UIBoxCreate(Label,
              UI_WidgetFlag_DrawText |
              UI_WidgetFlag_DrawBackground);
  GetUIInput();
}

//~ End UI Builder