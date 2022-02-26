/* date = November 15th 2021 10:47 pm */

#ifndef PHYSICS_SIM_BLAH_H
#define PHYSICS_SIM_BLAH_H

#include "physics_sim_types.h"
#include "physics_sim_math.h"
#include "physics_sim_memory.h"
#include "physics_sim_renderer.h"


#define TEST 1


enum entity_type
{
    Entity_Wall,
    Entity_Moves,
};

struct entity
{
    b32 Exists;
    
    entity_type Type;
    
    v3f32 Pos;
    v3f32 Vel;
    v3f32 Acc;
    
    v3f32 Dim; /// Unit: Meters
    
    f32 EulerX;
    f32 EulerY;
    f32 EulerZ;
};

struct app_state
{
    b32 IsInitialized;
    f32 DeltaTimeMS;
    f32 Time;
    
    memory_arena AssetArena;
    
    entity Entities[256];
    u32 EntityCount;
    u32 EntityMaxCount;
};

#define SIM_UPDATE( name) void name(app_memory *AppMemory, render_buffer *RenderBuffer)
typedef SIM_UPDATE(SIM_Update);
SIM_UPDATE(SIMUpdateStub)
{ return; }


#endif //PHYSICS_SIM_BLAH_H
