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