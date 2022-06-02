/* date = November 15th 2021 10:47 pm */

#ifndef PHYSICS_SIM_BLAH_H
#define PHYSICS_SIM_BLAH_H

#include "physics_sim_types.h"
#include "physics_sim_math.h"
#include "physics_sim_memory.h"
#include "physics_sim_renderer.h"


#define TEST 1


enum key_type
{
  Key_NULL,
  Key_a, Key_b, Key_c, Key_d,
  Key_e, Key_f, Key_g, Key_h,
  Key_i, Key_j, Key_k, Key_l,
  Key_m, Key_n, Key_o, Key_p,
  Key_q, Key_r, Key_s, Key_t,
  Key_u, Key_v, Key_w, Key_x,
  Key_y, Key_z,
  Key_MAX
};

struct button_state
{
  b32 EndedDown;
  u32 HalfTransitionCount;
};

struct app_input
{
  button_state AlphaKeys[Key_MAX]; 
  button_state NavKeys[Key_MAX]; // NOTE(MIGUEL): Space for extra keys
  button_state ConnectArduino; 
  button_state SpawnGraph;
  v2f Mouse; // NOTE(MIGUEL): What are the downsides to handleing screen coords with floatingpoint
};

enum entity_type
{
  Entity_Wall,
  Entity_Moves,
};

struct entity
{
  b32 Exists;
  
  entity_type Type;
  
  v3f Pos;
  v3f Vel;
  v3f Acc;
  
  v3f Dim; /// Unit: Meters
  
  f32 EulerX;
  f32 EulerY;
  f32 EulerZ;
};

enum dbg_cycle_counter_type
{
  DBG_CycleCounter_Update,
  DBG_CycleCounter_Render,
  DBG_CycleCounter_Count,
};

struct dbg_cycle_counter
{
  u64 CycleCount;
  u32 HitCount;
};

#if _MSC_VER
#define BEGIN_TIMED_BLOCK_(StartCycleCountWithID) StartCycleCountWithID = __rdtsc()
#define BEGIN_TIMED_BLOCK(ID) u64 BEGIN_TIMED_BLOCK_(StartCycleCount##ID)
#define END_TIMED_BLOCK_(StartCycleCountWithID, ID) \
GlobalDebugState->Counter[DBG_CycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; \
GlobalDebugState->Counter[DBG_CycleCounter_##ID].HitCount++;
#define END_TIMED_BLOCK(ID) \
GlobalDebugState->Counter[DBG_CycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; \
GlobalDebugState->Counter[DBG_CycleCounter_##ID].HitCount++;

struct timed_block
{
  u64   TimeStamp;
  char *FileName;
  char *FunctionName;
};
//
//#define TIMED_BLOCK timed_block TimedBlock_##__LINE__(__FILE__, __LINE__, __FUNCTION);
//struct timed_block
//{
//u64 StartCycleCount;
//u32 Id;
//timed_block (char *FileName, u32 LineNum, char *FuncName) { BEGIN_TIMED_BLOCK_(StartCycleCount); }
//~timed_block(void  ) { END_TIMED_BLOCK_(StartCycleCount, Id); }
//};
//
#endif
enum axis2d
{
  Axis_X,
  Axis_Y,
  Axis_Count,
};

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

enum ui_key
{
  
};

enum ui_flags
{
  UI_WidgetFlag_Clickable       = (1<<0),
  UI_WidgetFlag_ViewScroll      = (1<<1),
  UI_WidgetFlag_DrawText        = (1<<2),
  UI_WidgetFlag_DrawBorder      = (1<<3),
  UI_WidgetFlag_DrawBackground  = (1<<4),
  UI_WidgetFlag_DrawDropShadow  = (1<<5),
  UI_WidgetFlag_Clip            = (1<<6),
  UI_WidgetFlag_HotAnimation    = (1<<7),
  UI_WidgetFlag_ActiveAnimation = (1<<8),
};

struct ui_block
{
  ui_block *First;
  ui_block *Last;
  ui_block *Next;
  ui_block *Prev;
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
};

struct ui_state
{
  //Rendering 
  render_buffer *RenderBuffer;
  
  //Parent ManagmentStack
  ui_block ParentStack[256];
  u32 ParentCount;
  u32 ParentMaxCount;
  ui_block *ActiveParent;
  
  ui_block *BlockRoot;
  memory_arena Arena;
};

global ui_state   GlobalUIState;

//
// basic key type helpers
//ui_key UIKeyNull(void);
//ui_key UI_KeyFromString(String8 string);
//b32 UIKeyMatch(UI_Key a, UI_Key b);
//
struct app_state
{
  b32 IsInitialized;
  f64 DeltaTimeMS;
  f64 LongestFrameTime;
  f64 Time;
  
  // NOTE(MIGUEL): Temp
  v2f WindowDim;
  
  
  memory_arena AssetArena;
  memory_arena TextArena ;
  memory_arena LineArena ;
  
  f32 MeterToPixels;
  entity Entities[256];
  u32 EntityCount;
  u32 EntityMaxCount;
  u32 DeadEntityCount;
  dbg_cycle_counter Counter[DBG_CycleCounter_Count];
};

app_state *GlobalDebugState = NULL;

#define SIM_UPDATE( name) void name(app_memory *AppMemory, app_input *AppInput, render_buffer *RenderBuffer)
typedef SIM_UPDATE(SIM_Update);
SIM_UPDATE(SIMUpdateStub)
{ return; }


#endif //PHYSICS_SIM_BLAH_H
