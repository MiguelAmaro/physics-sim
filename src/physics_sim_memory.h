/* date = September 23rd 2021 9:31 pm */

#ifndef PHYSICS_SIM_MEMORY_H
#define PHYSICS_SIM_MEMORY_H

#include <windows.h>
//#include "physics_sim_common.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
//#include <stdarg.h>
#include "physics_sim_types.h"
#include "physics_sim_math.h"
#include "physics_sim_platform.h"


#define PERMANENT_STORAGE_SIZE (MEGABYTES(256))
#define TRANSIENT_STORAGE_SIZE (GIGABYTES(  4))

struct str8
{
  u8 *Data;
  u64 Length;
};

struct app_memory
{
  size_t PermanentStorageSize;
  size_t TransientStorageSize;
  
  void *PermanentStorage;
  void *TransientStorage;
  
  void   *MainBlock;
  size_t  MainBlockSize;
};


struct memory_arena
{
  size_t  Size;
  size_t  Used;
  void   *BasePtr;
};


str8
Str8(u8 *String, u64 Length)
{
  str8 Result = {0};
  Result.Data = String;
  Result.Length = Length;
  return Result;
}

u32 CStrGetSize(u8 *String, b32 IncludeNull)
{
  u32 Result = 0;
  while(String[Result] && Result<U32MAX) { Result++; }
  Result += IncludeNull?1:0;
  return Result;
}

str8 Str8FromCStr(u8 *String)
{
  str8 Result = Str8(String, CStrGetSize(String, 0));
  return Result;
}

str8
Str8FormatFromArena(memory_arena *Arena, char const * Format, ...)
{
  str8 Result = {0};
  // NOTE(MIGUEL): UNSAFE FUNCTION!!! GOOD LUCK! ;)
  va_list Args;
  va_start(Args, Format);
  // NOTE(MIGUEL): this can possibly write outside the arena.
  Result.Data  = ((u8 *)Arena->BasePtr + Arena->Used);
  //STBSP__PUBLICDEF int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va)
  Result.Length = stbsp_vsprintf((char *)Result.Data, Format, Args);
  va_end(Args);
  
  if(Arena->Used + Result.Length < Arena->Size)
  {
    Arena->Used += Result.Length;
  }
  else
  {
    Assert(!"Wrote outside arena boundries");
  }
  
  return Result;
}

inline u32
SafeTruncateu64(u64 Value)
{
  Assert(Value <= 0xffffffff);
  
  u32 Result = (u32)Value;
  
  return Result;
}


// NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
//               results in a noticable slow down.
void MemorySet(u32 Value, void *Dst, size_t DstSize)
{
  u8 *Dst0 = (u8 *)Dst;
  
  while(DstSize--) {*Dst0++ = (u8)Value;}
  
  return;
}

void MemoryCopy(void *Src, size_t SrcSize, void *Dst, size_t DstSize)
{
  size_t MaxSize = Min(SrcSize, DstSize);
  Assert(MaxSize>0 && SrcSize<=DstSize);
  u8 *Src0 = (u8 *)Src;
  u8 *Dst0 = (u8 *)Dst;
  while(MaxSize--) {*Dst0++ = *Src0++;}
  return;
}

void
ArenaInit(memory_arena *Arena, size_t Size, void *BasePtr)
{
  Arena->BasePtr = BasePtr;
  Arena->Size    = Size;
  Arena->Used    = 0;
  return;
}

void
ArenaDiscard(memory_arena *Arena)
{
  // NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
  //               results in a noticable slow down.
  MemorySet(0, Arena->BasePtr, Arena->Used);
  
  Arena->BasePtr = 0;
  Arena->Size    = 0;
  Arena->Used    = 0;
  return;
}

void
ArenaReset(memory_arena *Arena)
{
  // NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
  //               results in a noticable slow down.
  MemorySet(0, Arena->BasePtr, Arena->Used);
  Arena->Used    = 0;
  return;
}

#define ArenaPushStruct(Arena,        Type) (Type *)MemoryArenaPushBlock(Arena, sizeof(Type))
#define ArenaPushArray( Arena, Count, Type) (Type *)MemoryArenaPushBlock(Arena, (Count) * sizeof(Type))
#define ArenaZeroStruct(Instance          )         MemoryArenaZeroBlock(sizeof(Instance), &(Instance))
inline void *
MemoryArenaPushBlock(memory_arena *Arena, size_t Size)
{
  Assert((Arena->Used + Size) <= Arena->Size);
  
  void *NewArenaPartitionAdress  = (u8 *)Arena->BasePtr + Arena->Used;
  Arena->Used  += Size;
  
  return NewArenaPartitionAdress;
}

inline void
MemoryArenaZeroBlock(memory_index size, void *address)
{
  u8 *byte = (u8 *)address;
  
  while(size--)
  {
    *byte++ = 0;
  }
  
  return;
}

#endif //PHYSICS_SIM_MEMORY_H
