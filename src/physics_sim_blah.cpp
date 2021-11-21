#include "physics_sim_blah.h"
#include "physics_sim_assets.h"
#include "windows.h"

static void SimInit(app_state *AppState)
{
    // NOTE(MIGUEL): Sim Initialization
    AppState->EntityCount    =   0;
    AppState->EntityMaxCount = 256;
    
    // NOTE(MIGUEL): Entity Generation
    
    
    // NOTE(MIGUEL): Wall Entities
    
#ifndef TEST
#endif
    entity *EntityWallLeft   = AppState->Entities + AppState->EntityCount++;
    entity *EntityWallRight  = AppState->Entities + AppState->EntityCount++;
    entity *EntityWallBottom = AppState->Entities + AppState->EntityCount++;
    entity *EntityWallTop    = AppState->Entities + AppState->EntityCount++;
    
    entity *Entity = nullptr;
    
    f32 CommonWidth = 40.0f;
    
    f32 SpaceWidth  = 400.0f;
    f32 SpaceHeight = 400.0f;
#ifndef TEST
#endif
    Entity = EntityWallLeft;
    Entity->Dim.x = CommonWidth;
    Entity->Dim.y = SpaceHeight + (CommonWidth * 2.0f);
    Entity->Dim.z = CommonWidth;
    
    Entity->Pos.x = -1.0f * (SpaceWidth / 2.0f) - (Entity->Dim.x / 2.0f);
    Entity->Pos.y = 0.0f;
    Entity->Pos.z = 0.0f;
    Entity->Type = Entity_Wall;
    Entity->Exists = true;
    
    Entity = EntityWallRight;
    Entity->Dim.x = CommonWidth;
    Entity->Dim.y = SpaceHeight + (CommonWidth * 2.0f);
    Entity->Dim.z = CommonWidth;
    
    Entity->Pos.x = 1.0f * (SpaceWidth / 2.0f) + (Entity->Dim.x / 2.0f);
    Entity->Pos.y = 0.0f;
    Entity->Pos.z = 0.0f;
    Entity->Type = Entity_Wall;
    Entity->Exists = true;
    Entity = EntityWallTop;
    Entity->Dim.x = SpaceWidth + 0.0f;
    Entity->Dim.y = CommonWidth; 
    Entity->Dim.z = CommonWidth;
    
    Entity->Pos.x = 0.0f;
    Entity->Pos.y = 1.0f * (SpaceHeight / 2.0f) + (Entity->Dim.y / 2.0f);
    Entity->Pos.z = 0.0f;
    Entity->Type = Entity_Wall;
    Entity->Exists = true;
    
#ifndef TEST
#endif
    Entity = EntityWallBottom;
    Entity->Dim.x = SpaceWidth + 0.0f;
    Entity->Dim.y = CommonWidth; 
    Entity->Dim.z = CommonWidth;
    
    Entity->Pos.x = 0.0f;
    Entity->Pos.y = -1.0f * (SpaceHeight / 2.0f) - (Entity->Dim.y / 2.0f);
    Entity->Pos.z = 0.0f;
    Entity->Type = Entity_Wall;
    Entity->Exists = true;
    
    // NOTE(MIGUEL): Moving Entities
    u32 EntitiesWanted = 100;
    u32 VectorTableSize = ARRAY_SIZE(NormalizedVectorTable);
    for(u32 EntityIndex = 0; EntityIndex < EntitiesWanted; EntityIndex++)
    {
        
        if(AppState->EntityCount < AppState->EntityMaxCount)
        {
            Entity = AppState->Entities + AppState->EntityCount++;
            
            //SYSTEMTIME SysTime;
            //GetSystemTime(&SysTime);
            
            LARGE_INTEGER Counter;
            QueryPerformanceCounter(&Counter);
            
            u32 Random = Counter.LowPart;
            
            u32 SeedX = (Random + 4 * (EntityIndex << 3)) << 8;
            u32 SeedY = (Random * (AppState->EntityCount << 23)) << 3;
            f32 X = NormalizedVectorTable[SeedX % VectorTableSize];
            f32 Y = NormalizedVectorTable[SeedY % VectorTableSize];
            
#ifdef TEST
            
            Entity->Pos.x =  0.0f;
            Entity->Pos.y =  0.0f;
            Entity->Pos.z =  0.0f;
#else
            Entity->Pos.x =  (24 * EntityIndex) % (u32)SpaceWidth;
            Entity->Pos.y =  (24 * EntityIndex) % (u32)SpaceHeight;
            Entity->Pos.z =  0.0f;
#endif
            Entity->Dim.x = 20.0f;
            Entity->Dim.y = 20.0f; 
            Entity->Dim.z = 20.0f;
            
            Entity->Acc.x = 0.0f;
            Entity->Acc.y = 0.0f;
            Entity->Acc.z = 0.0f;
            
#ifndef TEST
            Entity->Vel.x = X;
            Entity->Vel.y = Y;
            Entity->Vel.z = 0.0f;
#else
            Entity->Vel.x = 0.3251f;
            Entity->Vel.y = 0.47999999f;
            Entity->Vel.z = 0.0f;
#endif
            
            Entity->Type = Entity_Moves;
            Entity->Exists = true;
        }
    };
    
    return;
}

static b32
Intersects(f32 *NearestNormalizedCollisionPoint,
           f32 WallA     ,
           f32 RelPosA   , f32 RelPosB,
           f32 PosDeltaA , f32 PosDeltaB,
           f32 MinB      , f32 MaxB)
{
    b32 Result    = 0;
    f32 T_Epsilon = 0.0001f;
    
    if(PosDeltaA != 0.0f)
    {
        f32 NormalizedCollisionPoint = (WallA - RelPosA) / PosDeltaA;
        f32 PointOfCollisionB        = RelPosB + (PosDeltaB * NormalizedCollisionPoint);
        
        if((NormalizedCollisionPoint >= 0.0f) &&
           (*NearestNormalizedCollisionPoint > NormalizedCollisionPoint))
        {
            if((PointOfCollisionB >= MinB) && (PointOfCollisionB <= MaxB));
            {
                *NearestNormalizedCollisionPoint = MAXIMUM(0.0f,
                                                           NormalizedCollisionPoint - T_Epsilon);
                
                Result = 1;
            }
            
        }
    }
    
    return Result;
}

extern "C" SIM_UPDATE(Update)
{
    app_state *AppState = (app_state *)AppMemory->PermanentStorage;
    
    if(AppState->IsInitialized == false)
    {
        SimInit(AppState);
        AppState->IsInitialized = true;
    }
    
    static f32 RotZ = 0.0f;
    RotZ += 0.2f;
    
    entity   *Entity   =  AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
        if(Entity->Type == Entity_Moves && Entity->Exists)
        {
#ifndef TEST
#else
            // NOTE(MIGUEL): Equations of motion
            if(Entity->Type == Entity_Moves)
            {
                int dbgint = 13;
                dbgint = 1408;
            }
#endif
            f32   Speed = 0.25f;
            v3f32 Drag  = {0.0f, 0.0f, 0.0f};
            v3f32 Acc   = Entity->Acc * 1.0f;
            v3f32 Vel   = Speed * (Entity->Vel + Drag);
            
            v3f32 PosDelta = (0.5f * Acc * Square(AppState->DeltaTimeMS) +
                              Vel * AppState->DeltaTimeMS);
            
            Entity->Vel = Acc * AppState->DeltaTimeMS + Entity->Vel;
            
            
            v3f32 NormalizedVel = v3f32Normalize(Vel);
            Entity->EulerZ = ArcTan2(NormalizedVel.x, NormalizedVel.y);
            f32   NormalizedCollisionPoint = 1.0f;
            f32 RemainingTravelDistance = v3f32GetMagnitude(PosDelta);
            
            for(u32 Iteration = 0; Iteration < 4; Iteration++)
            {
                entity *TestEntity = AppState->Entities;
                for(u32 TestEntityIndex = 0; TestEntityIndex < AppState->EntityCount;
                    TestEntityIndex++, TestEntity++)
                {
                    if(TestEntity != Entity &&
                       TestEntity->Type == Entity_Wall &&
                       TestEntity->Exists)
                    {
                        /// TEST ENTITY SPACE
                        v2f32 EntityPos = Entity->Pos.xy - TestEntity->Pos.xy;
                        
                        rect_v2f32 TestEntityBounds = rect_v2f32CenteredDim(TestEntity->Dim.xy) ;
                        
                        if(rect_v2f32IsInside(TestEntityBounds, EntityPos))
                        {
                            Entity->Exists = false;
                        }
                        
                        ASSERT(Entity->Exists);
                        
                        rect_v2f32 MinkowskiTestEntityBounds = { 0 };
                        MinkowskiTestEntityBounds = rect_v2f32AddRadiusTo(TestEntityBounds,
                                                                          Entity->Dim.xy);
                        v3f32 WallNormal = { 0 };
                        
                        entity *HitEntity  = nullptr;
                        // NOTE(MIGUEL): Intersection Test on all the walls of Test entity
                        // LEFT WALL
                        if(Intersects(&NormalizedCollisionPoint,
                                      MinkowskiTestEntityBounds.min.x,
                                      EntityPos.x, EntityPos.y,
                                      PosDelta.x , PosDelta.y,
                                      MinkowskiTestEntityBounds.min.y,
                                      MinkowskiTestEntityBounds.max.y))
                        {
                            WallNormal = v3f32Init(-1.0f, 0.0f, 0.0f);
                            HitEntity = TestEntity;
                        }
                        // RIGHT WALL
                        if(Intersects(&NormalizedCollisionPoint,
                                      MinkowskiTestEntityBounds.max.x,
                                      EntityPos.x, EntityPos.y,
                                      PosDelta.x , PosDelta.y,
                                      MinkowskiTestEntityBounds.min.y,
                                      MinkowskiTestEntityBounds.max.y))
                        {
                            WallNormal = v3f32Init(1.0f, 0.0f, 0.0f);
                            HitEntity = TestEntity;
                        }
                        // TOP WALL
                        if(Intersects(&NormalizedCollisionPoint,
                                      MinkowskiTestEntityBounds.max.y,
                                      EntityPos.y, EntityPos.x,
                                      PosDelta.y , PosDelta.x,
                                      MinkowskiTestEntityBounds.min.x,
                                      MinkowskiTestEntityBounds.max.x))
                        {
                            WallNormal = v3f32Init(0.0f, 1.0f, 0.0f);
                            HitEntity = TestEntity;
                        }
                        // BOTTOM WALL
                        if(Intersects(&NormalizedCollisionPoint,
                                      MinkowskiTestEntityBounds.min.y,
                                      EntityPos.y, EntityPos.x,
                                      PosDelta.y , PosDelta.x,
                                      MinkowskiTestEntityBounds.min.x,
                                      MinkowskiTestEntityBounds.max.x))
                        {
                            WallNormal = v3f32Init(0.0f, -1.0f, 0.0f);
                            HitEntity = TestEntity;
                        }
                        
                        //RemainingTravelDistance = ;
                        // NOTE(MIGUEL): Computes New Pos Delta On Collision
                        if(HitEntity)
                        {
                            v3f32 NewVel = WallNormal;
                            f32   VelDot = v3f32Inner(Entity->Vel, WallNormal);
                            Entity->Vel -= NewVel * VelDot;
                            Entity->Vel -= NewVel * VelDot;
                            
                            
                            v3f32 NewPosDelta = WallNormal;
                            f32   PosDeltaDot = v2f32Inner(PosDelta.xy, WallNormal.xy);
                            PosDelta -= NewVel * PosDeltaDot;
                            PosDelta -= NewVel * PosDeltaDot;
                            
                        }
                    }
                }
#ifndef TEST
                Entity->Pos += PosDelta;
#else
                Entity->Pos += PosDelta;
                f32 SpaceWidth  = 400.0f;
                f32 SpaceHeight = 400.0f;
                ASSERT(Entity->Pos.x <= SpaceWidth &&
                       Entity->Pos.y <= SpaceHeight);
                
#endif
                
            }
        }
    }
    
    
    return;
}
