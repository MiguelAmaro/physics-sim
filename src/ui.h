/* date = July 15th 2022 5:19 pm */

#ifndef UI_H
#define UI_H

enum ui_size_opt
{
  UI_Size_Null,
  UI_Size_Pixels,
  UI_Size_TextContent,
  UI_Size_PercentOfParent,
  UI_Size_ChildrenSum,
};

struct ui_size
{
  ui_size_opt Option;
  f32 Value;
  f32 Strictness;
};

union ui_key
{
  void *Address;
  u64   Key;
};

struct ui_input
{
  b32 Hover;
  b32 Clicked;
};

enum ui_flags
{
  UI_BlockFlag_Clickable       = (1<<0),
  UI_BlockFlag_ViewScroll      = (1<<1),
  UI_BlockFlag_DrawText        = (1<<2),
  UI_BlockFlag_DrawBorder      = (1<<3),
  UI_BlockFlag_DrawBackground  = (1<<4),
  UI_BlockFlag_DrawDropShadow  = (1<<5),
  UI_BlockFlag_Clip            = (1<<6),
  UI_BlockFlag_HotAnimation    = (1<<7),
  UI_BlockFlag_ActiveAnimation = (1<<8),
};

struct ui_block
{
  ui_block *FirstChild;
  ui_block *LastChild;
  ui_block *NextSibling;
  ui_block *PrevSibling;
  ui_block *Parent;
  //Hash Links
  ui_block *HashNext;
  ui_block *HashPrev;
  //Key+Gen Info
  ui_key Key;
  u64 LastFrameTouchedIndex;
  ui_size Size[Axis_Count];
  
  //Perframe
  ui_flags Flags;
  str8 String;
  v2f ComputedRelPos;
  v2f ComputedSize;
  r2f Rect;
  
  //Persistant
  f32 Hot;
  f32 Active;
  
  //Test
  v4f Color;
};

struct ui_state
{
  //Rendering 
  render_buffer *RenderBuffer;
  v2f WindowDim;
  
  //Parent ManagmentStack
  ui_block ParentStack[256];
  u32 ParentCount;
  u32 ParentMaxCount;
  ui_block *ActiveParent;
  
  app_input Input;
  // NOTE(MIGUEL): Reading ryaans ui guide the usual amount of widgets used at any given moment is ~400
  //               1000 widgets is an upper bound. Im allocating mem upfront for the worst case but 
  //               i might want to allocate as i go. I'm choosing not to because i dont thint any extra
  //               complecity is worth it.
  ui_block  UIBlockHash[1049];
  u32       UIBlockHashCount;
  u32       UIBlockHashMaxCount;
  arena Arena;
};

global ui_state *GlobalUIState;

#endif //UI_H
