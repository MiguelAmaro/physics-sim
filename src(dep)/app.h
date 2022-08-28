#ifndef APP_H
#define APP_H

#include "types.h"
#include "math.h"
#include "memory.h"
#include "renderer.h"



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
  v2f MousePos; // NOTE(MIGUEL): What are the downsides to handleing screen coords with floatingpoint
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

struct app_state
{
  b32 IsInitialized;
  f64 DeltaTimeMS;
  f64 LongestFrameTime;
  f64 Time;
  
  // NOTE(MIGUEL): Temp
  v2f WindowDim;
  
  //Frame Data
  //ui_block Hash[1049];
  arena AssetArena;
  arena TextArena ;
  arena LineArena ;
  
  f32 MeterToPixels;
  entity Entities[256];
  u32 EntityCount;
  u32 EntityMaxCount;
  u32 DeadEntityCount;
  //ui_state UIState;
  dbg_cycle_counter Counter[DBG_CycleCounter_Count];
};

app_state *GlobalDebugState = NULL;

#define SIM_UPDATE( name) void name(app_memory *AppMemory, app_input *AppInput, render_buffer *RenderBuffer)
typedef SIM_UPDATE(SIM_Update);
SIM_UPDATE(SIMUpdateStub)
{ return; }


#endif //APP_H
