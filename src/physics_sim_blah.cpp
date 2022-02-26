#include "physics_sim_blah.h"
#include "physics_sim_assets.h"
#include "physics_sim_assets.h"
#include "windows.h"

static void SimInit(app_state *AppState)
{
    // NOTE(MIGUEL): Sim Initialization
    AppState->EntityCount    =   0;
    AppState->EntityMaxCount = 256;
    
    // NOTE(MIGUEL): Entity Generation
    
    
    // NOTE(MIGUEL): Wall Entities
    
#if TEST
#endif
    entity *EntityWallLeft   = AppState->Entities + AppState->EntityCount++;
    entity *EntityWallRight  = AppState->Entities + AppState->EntityCount++;
    entity *EntityWallBottom = AppState->Entities + AppState->EntityCount++;
    entity *EntityWallTop    = AppState->Entities + AppState->EntityCount++;
    
    entity *Entity = nullptr;
    
    f32 CommonWidth = 40.0f;
    
    f32 SpaceWidth  = 400.0f;
    f32 SpaceHeight = 400.0f;
    
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
    u32 EntitiesWanted = 1;
    u32 VectorTableSize = ARRAY_COUNT(NormalizedVectorTable);
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
            
            Entity->Pos.x =  0.0f;
            Entity->Pos.y =  0.0f;
            Entity->Pos.z =  0.0f;
            
            Entity->Dim.x = 20.0f;
            Entity->Dim.y = 20.0f; 
            Entity->Dim.z = 20.0f;
            
            Entity->Acc.x = 0.0f;
            Entity->Acc.y = 0.0f;
            Entity->Acc.z = 0.0f;
            
            Entity->Vel.x = 0.3251f;
            Entity->Vel.y = 0.47999999f;
            Entity->Vel.z = 0.0f;
            
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

void
DrawVec(render_buffer *RenderBuffer, v3f32 Vec, v3f32 Pos)
{
    v3f32 CosSin = v3f32Normalize(Vec);
    v3f32 Dim    = v3f32Init(400.0f *v3f32Magnitude(Vec), 1.0f, 1.0f);
    
    PushLine(RenderBuffer, Pos, Dim, CosSin);
    
    return;
}

void
DrawAABB(render_buffer *RenderBuffer, entity Entity)
{
    v3f32 Origin = Entity.Pos;
    v3f32 Offset = Entity.Dim / 2.0f;
    
    v3f32 PosL = v3f32Init(Origin.x - Offset.x, Origin.y, Origin.z);
    v3f32 PosR = v3f32Init(Origin.x + Offset.x, Origin.y, Origin.z);
    v3f32 PosT = v3f32Init(Origin.x, Origin.y + Offset.y, Origin.z);
    v3f32 PosB = v3f32Init(Origin.x, Origin.y - Offset.y, Origin.z);
    
    v3f32 DimLR = v3f32Init(Entity.Dim.y, 1.0f,  1.0f);
    v3f32 DimTB = v3f32Init(1.0f, Entity.Dim.x, 1.0f);
    
    v3f32 CosSin = v3f32Normalize(Entity.Vel);
    PushRect(RenderBuffer, PosL, DimLR, v3f32Init(0.0, 0.0,0.0));
    PushRect(RenderBuffer, PosR, DimLR, v3f32Init(0.0, 0.0,0.0));
    //PushRect(RenderBuffer, PosT, DimTB, v3f32Init(1.0, 1.0,0.0));
    //PushRect(RenderBuffer, PosB, DimTB, v3f32Init(0.0, 1.0,0.0));
    
    return;
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
#if TEST
#else
            // NOTE(MIGUEL): Equations of motion
            if(Entity->Type == Entity_Moves)
            {
                int dbgint = 13;
                dbgint = 1408;
            }
#endif
            f32   Speed = 0.03f;
            v3f32 Drag  = {0.0f, 0.0f, 0.0f};
            v3f32 Acc   = Entity->Acc * 1.0f;
            v3f32 Vel   = Speed * (Entity->Vel + Drag);
            
            v3f32 PosDelta = (0.5f * Acc * Square(AppState->DeltaTimeMS) +
                              Vel * AppState->DeltaTimeMS);
            
            Entity->Vel = Acc * AppState->DeltaTimeMS + Entity->Vel;
            
            //Entity->EulerZ = ArcTan2(Vel.y, Vel.x);
            
            
            f32 NormalizedCollisionPoint = 1.0f;
            f32 RemainingTravelDistance = v3f32Magnitude(PosDelta);
            
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
                        
                        //DrawLine(RenderBuffer, Point1, Point2);
                        
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
                
#if TEST
                Entity->Pos += PosDelta;
#else
                Entity->Pos += PosDelta;
                f32 SpaceWidth  = 400.0f;
                f32 SpaceHeight = 400.0f;
                /*ASSERT(Entity->Pos.x <= SpaceWidth &&
                       Entity->Pos.y <= SpaceHeight);*/
                
#endif
                
            }
        }
    }
    
    Entity =  AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
        DrawAABB(RenderBuffer, *Entity);
        PushRect(RenderBuffer, Entity->Pos, Entity->Dim, Entity->Vel);
    }
    
    
    return;
}
