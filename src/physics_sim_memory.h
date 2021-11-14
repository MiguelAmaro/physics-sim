/* date = September 23rd 2021 9:31 pm */

#ifndef PHYSICS_SIM_MEMORY_H
#define PHYSICS_SIM_MEMORY_H


#include "physics_sim_types.h"
#include "physics_sim_math.h"
#include "physics_sim_platform.h"


#define PERMANENT_STORAGE_SIZE (MEGABYTES(256))
#define TRANSIENT_STORAGE_SIZE (GIGABYTES(  4))

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

void
MemoryArenaInit(memory_arena *Arena, size_t Size, void *BasePtr)
{
    Arena->BasePtr = BasePtr;
    Arena->Size    = Size;
    Arena->Used    = 0;
    
    return;
}

#define MEMORY_ARENA_PUSH_STRUCT(Arena,        Type) (Type *)MemoryArenaPushBlock(Arena, sizeof(Type))
#define MEMORY_ARENA_PUSH_ARRAY( Arena, Count, Type) (Type *)MemoryArenaPushBlock(Arena, (Count) * sizeof(Type))
#define MEMORY_ARENA_ZERO_STRUCT(Instance          )         MemoryArenaZeroBlock(sizeof(Instance), &(Instance))
inline void *
MemoryArenaPushBlock(memory_arena *Arena, size_t Size)
{
    ASSERT((Arena->Used + Size) <= Arena->Size);
    
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
