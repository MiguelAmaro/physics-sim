#include "physics_sim_blah.h"
#include "physics_sim_assets.h"
#include "physics_sim_assets.h"
#include "windows.h"

#define QUADTREE_CAP (4)
struct qtree
{
  r2f Bounds;
  
  qtree *TopLeft;
  qtree *TopRight;
  qtree *BotLeft;
  qtree *BotRight;
};

void QtreeInsert()
{
  return;
}

void QtreeSubdivide()
{
  return;
}

void QtreeQueryRange()
{
  return;
}

static void SimInit(app_state *AppState)
{
  
  AppState->MeterToPixels = 20.0;
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
  // TODO(MIGUEL): Convert from pixels to meters
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
  u32 EntityIndex = 0;
  f32 EntityDim = 20.0f;
  u32 EntitiesWanted = 10;
  u32 VectorTableSize = ARRAY_COUNT(NormalizedVectorTable);
  
  f32 TurnFraction= GOLDEN_RATIO32;
  u32 Resolution = 1;
  f32 NumPoints = 100;
  f32 Radius = 400.0f-40.0f;
  for(u32 Index=0; Index<NumPoints; Index++)
  {
    f32 Dist = Root((f32)Index/(NumPoints - 1.0f));
    f32 Theta = 2.0f*PI32*TurnFraction*(f32)Index;
    
    v3f32 Point = v3f32Init(Dist*Cosine(Theta),
                            Dist*Sine  (Theta), 0.0f); 
    
    
    //for(u32 EntityIndex = 0; EntityIndex < EntitiesWanted; EntityIndex++)
    f32 ClosetDistance = 0;
    if(AppState->EntityCount>0)
    {
      v3f32 Delta = {0};
      entity *TestEntity = AppState->Entities + (AppState->EntityCount-1);
      for(u32 TestEntityIndex=0;
          TestEntityIndex<AppState->EntityMaxCount;
          TestEntityIndex++, TestEntity++)
      {
        Delta = (Radius*Point)+ -1.0*TestEntity->Pos;
        ClosetDistance = v3f32Magnitude(Delta);
      }
    }
    
    if((ClosetDistance>EntityDim) &&
       (EntityIndex<EntitiesWanted) &&
       (AppState->EntityCount<AppState->EntityMaxCount))
    {
      Entity = AppState->Entities + AppState->EntityCount++;
      
      
      LARGE_INTEGER Counter;
      QueryPerformanceCounter(&Counter);
      u32 Random = Counter.LowPart;
      
      u32 SeedX = (Random + 4 * (EntityIndex << 3)) << 8;
      u32 SeedY = (Random * (AppState->EntityCount << 23)) << 3;
      f32 DirX = NormalizedVectorTable[SeedX % VectorTableSize];
      f32 DirY = NormalizedVectorTable[SeedY % VectorTableSize];
      
      Entity->Pos.x =  Radius*Point.x;
      Entity->Pos.y =  Radius*Point.y;
      Entity->Pos.z =  0.0f;
      
      Entity->Dim.x = EntityDim;
      Entity->Dim.y = EntityDim; 
      Entity->Dim.z = 20.0f;
      
      Entity->Acc.x = 0.0f;
      Entity->Acc.y = 0.0f;
      Entity->Acc.z = 0.0f;
      
      // NOTE(MIGUEL): Is normalize broken ?
      Entity->Vel = v3f32Normalize(v3f32Init(DirX, DirY, 0.0f));
      Entity->Vel.z = 0.0f;
      
      Entity->Type = Entity_Moves;
      Entity->Exists = true;
      
      EntityIndex++;
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
      if((PointOfCollisionB >= MinB) && (PointOfCollisionB <= MaxB))
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
DrawPath(render_buffer *RenderBuffer, v3f32 *PointList, u32 PointCount, v4f32 Color)
{
  
  PushLine(RenderBuffer,
           v3f32Init(0.0f, 0.0f, 0.0f),
           v3f32Init(0.0f, 0.0f, 0.0f),
           PointList, PointCount, 2.0f, 0.9f, Color);
  
  return;
}

void
DrawVector(render_buffer *RenderBuffer, v3f32 Origin, v3f32 Vec, memory_arena *LineArena)
{
  v3f32 *PointList = MEMORY_ARENA_PUSH_ARRAY(LineArena, 2, v3f32);
  
  v4f32 Color  = v4f32Init(1.0f, 1.0f, 1.0f, 1.0f);
  
  PointList[0] = v3f32Init(Origin.x + 0.0f , Origin.y + 0.0f , 0.5);
  PointList[1] = v3f32Init(Origin.x + Vec.x, Origin.y + Vec.y, 0.5);
  
  PushLine(RenderBuffer,
           v3f32Init(0.0f, 0.0f, 0.0f),
           v3f32Init(0.0f, 0.0f, 0.0f),
           PointList, 2, 4.0f, 0.6f, Color);
  
  return;
}

void
DrawPerimeter(render_buffer *RenderBuffer, v3f32 Origin, rect_v2f32 Rect, memory_arena *LineArena)
{
  v4f32 Color  = v4f32Init(1.0f, 0.4f, 1.0f, 1.0f);
  u32 PointCount = 6;
  v3f32 *PointList = MEMORY_ARENA_PUSH_ARRAY(LineArena, PointCount, v3f32);
  
  PointList[0] = v3f32Init(Origin.x + Rect.max.x, Origin.y + Rect.max.y, 0.5);
  PointList[1] = v3f32Init(Origin.x + Rect.max.x, Origin.y + Rect.min.y, 0.5);
  PointList[2] = v3f32Init(Origin.x + Rect.min.x, Origin.y + Rect.min.y, 0.5);
  PointList[3] = v3f32Init(Origin.x + Rect.min.x, Origin.y + Rect.max.y, 0.5);
  PointList[4] = v3f32Init(Origin.x + Rect.max.x, Origin.y + Rect.max.y, 0.5);
  // NOTE(MIGUEL): This would be uneccesary if i implement round line caps
  PointList[5] = v3f32Init(Origin.x + Rect.max.x, Origin.y + Rect.min.y, 0.5);
  
  PushLine(RenderBuffer,
           v3f32Init(0.0f, 0.0f, 0.0f),
           v3f32Init(0.0f, 0.0f, 0.0f),
           PointList, PointCount, 3.0f, 0.2f, Color);
  return;
}

void
DrawAABB(render_buffer *RenderBuffer, entity *Entity, memory_arena *LineArena)
{
  v3f32 Origin = Entity->Pos;
  v3f32 Offset = Entity->Dim / 2.0f;
  
  v4f32 Color  = v4f32Init(1.0f, 1.0f, 1.0f, 1.0f);
  u32 PointCount = 6;
  v3f32 *PointList = MEMORY_ARENA_PUSH_ARRAY(LineArena, PointCount, v3f32);
  
  PointList[0] = v3f32Init(Origin.x + Offset.x, Origin.y + Offset.y, 0.5);
  PointList[1] = v3f32Init(Origin.x + Offset.x, Origin.y - Offset.y, 0.5);
  PointList[2] = v3f32Init(Origin.x - Offset.x, Origin.y - Offset.y, 0.5);
  PointList[3] = v3f32Init(Origin.x - Offset.x, Origin.y + Offset.y, 0.5);
  PointList[4] = v3f32Init(Origin.x + Offset.x, Origin.y + Offset.y, 0.5);
  // NOTE(MIGUEL): This would be uneccesary if i implement round line caps
  PointList[5] = v3f32Init(Origin.x + Offset.x, Origin.y - Offset.y, 0.5);
  
  PushLine(RenderBuffer,
           v3f32Init(0.0f, 0.0f, 0.0f),
           v3f32Init(0.0f, 0.0f, 0.0f),
           PointList, PointCount, 10.0f, 0.8f, Color);
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
  
  
  memory_arena TextArena = { 0 };
  MemoryArenaInit(&TextArena, KILOBYTES(3), AppMemory->TransientStorage);
  memory_arena LineArena = { 0 };
  MemoryArenaInit(&LineArena, KILOBYTES(20), (u8 *)AppMemory->TransientStorage + TextArena.Size);
  
  
  entity   *Entity   =  AppState->Entities;
  for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
  {
    if(Entity->Type == Entity_Moves && Entity->Exists)
    {
      f32   Speed = 0.03f;
      v3f32 Drag  = {0.0f, 0.0f, 0.0f};
      v3f32 Acc   = Entity->Acc * 1.0f;
      v3f32 Vel   = Speed * (Entity->Vel + Drag);
      
      v3f32 PosDelta = (0.5f * Acc * Square(AppState->DeltaTimeMS) +
                        Vel * AppState->DeltaTimeMS);
      
      Entity->Vel = Acc * AppState->DeltaTimeMS + Entity->Vel;
      
      u32 MaxCollisonIterations = 4;
      f32 NormalizedCollisionPoint = 1.0f;
      f32 RemainingTravelDistance = v3f32Magnitude(PosDelta);
      
      u32 Iteration = 0;
      for(; Iteration < MaxCollisonIterations; Iteration++)
      {
        entity *TestEntity = AppState->Entities;
        for(u32 TestEntityIndex = 0; TestEntityIndex < AppState->EntityCount;
            TestEntityIndex++, TestEntity++)
        {
          if(TestEntity != Entity &&
             (TestEntity->Type == Entity_Wall ||
              TestEntity->Type == Entity_Moves) &&
             TestEntity->Exists)
          {
            /// TEST ENTITY SPACE
            v2f32 EntityPos = Entity->Pos.xy - TestEntity->Pos.xy;
            
            rect_v2f32 TestEntityBounds = rect_v2f32CenteredDim(TestEntity->Dim.xy);
            
            
            if(rect_v2f32IsInside(TestEntityBounds, EntityPos))
            {
              Entity->Exists = false;
              goto ProcessNextEntity;
            }
            
            ASSERT(Entity->Exists == true);
            
            rect_v2f32 MinkowskiTestEntityBounds = { 0 };
            MinkowskiTestEntityBounds = rect_v2f32AddRadiusTo(TestEntityBounds,
                                                              Entity->Dim.xy);
            
            if(Iteration==0)
            {
              v3f32 EToTEDelta = v3f32Init(EntityPos.x, EntityPos.y, 0.5);
              DrawVector(RenderBuffer,Entity->Pos + 300.0f, -1.0f * EToTEDelta, &LineArena);
              rect_v2f32 MTB = {0};
              MTB.min = MinkowskiTestEntityBounds.min + 300.0f;
              MTB.max = MinkowskiTestEntityBounds.max + 300.0f;
              //pDrawPerimeter(RenderBuffer, TestEntity->Pos, MTB, &LineArena);
            }
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
        
        v3f32 OldPos = Entity->Pos;
        Entity->Pos.x += 300;
        Entity->Pos.y += 300;
        
        Entity->Pos.x = OldPos.x;
        Entity->Pos.y = OldPos.y;
        
      }
#if 0
      
      v3f32 *CollisionPoints = MEMORY_ARENA_PUSH_ARRAY(&LineArena, MaxCollisonIterations, v3f32);
      // NOTE(MIGUEL): 
      CollisionPoints[Iteration] = v3f32Init(Entity->Pos.x, Entity->Pos.y, 0.5);
      DrawVector(RenderBuffer, Vel, Entity->Pos, &LineArena);
      
      u32 CollisionCount = Iteration;
      v4f32 Color  = v4f32Init(1.0f, 1.0f, 1.0f, 1.0f);
      DrawPath(RenderBuffer, CollisionPoints, CollisionCount, Color);
#endif
    }
    
    ProcessNextEntity:
    u8 blah = 1;
  }
  
  Entity =  AppState->Entities;
  for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
  {
    if(Entity->Exists)
    {
      // TODO(MIGUEL): 
      v3f32 OldPos = Entity->Pos;
      Entity->Pos.x += 300;
      Entity->Pos.y += 300;
      
      DrawAABB(RenderBuffer, Entity, &LineArena);
      PushRect(RenderBuffer, Entity->Pos, Entity->Dim, Entity->Vel);
      
      Entity->Pos.x = OldPos.x;
      Entity->Pos.y = OldPos.y;
    }
    
  }
  
  
  return;
}
