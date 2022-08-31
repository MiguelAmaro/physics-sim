

#include "types.h"
#include "memory.h"
#include "string.h"
#include "os.h"
#include "math.h"

#include "renderer.h"
#include "app.h"

#include "assets.h"
#include "windows.h"
#include "win32_core.c"
#include "memory.c"
#include "string.c"

#define QUADTREE_CAP (4)
typedef struct qtree qtree;
struct qtree
{
  i2f Bounds;
  
  qtree *TopLeft;
  qtree *TopRight;
  qtree *BotLeft;
  qtree *BotRight;
};

void QtreeInsert(void)
{
  return;
}

void QtreeSubdivide(void)
{
  return;
}

void QtreeQueryRange(void)
{
  return;
}

void CreateMovableGroup(app_state *AppState )
{
  
  // NOTE(MIGUEL): Moving Entities
  u32 EntityIndex = 0;
  f32 EntityDim = 1.0f;
  u32 VectorTableSize = ArrayCount(NormalizedVectorTable);
  
  f32 TurnFraction= Golden32;
  //u32 Resolution = 1;
  f32 NumPoints = 10;
  f32 Radius = 10.0f;
  for(u32 Index=0; Index<(u32)NumPoints; Index++)
  {
    f32 Dist = SqrRoot((f32)Index/(NumPoints - 1.0f));
    f32 Theta = 2.0f*Pi32*TurnFraction*(f32)Index;
    
    v3f Point = V3f(Dist*Cosine(Theta)*Radius,
                    Dist*Sine  (Theta)*Radius, 0.0f); 
    
    
    //for(u32 EntityIndex = 0; EntityIndex < EntitiesWanted; EntityIndex++)
    f32 ClosetDistance = 0;
    if(AppState->EntityCount>0)
    {
      v3f Delta = {0};
      entity *TestEntity = AppState->Entities + (AppState->EntityCount-1);
      for(u32 TestEntityIndex=0;
          TestEntityIndex<AppState->EntityMaxCount;
          TestEntityIndex++, TestEntity++)
      {
        Delta = Add(Scale(Point,Radius), Scale(TestEntity->Pos, -1.0));
        ClosetDistance = Length(Delta);
      }
    }
    
    //if((ClosetDistance>EntityDim) &&
    //(EntityIndex<EntitiesWanted) &&
    entity *Entity   =  AppState->Entities;
    if(AppState->EntityCount<AppState->EntityMaxCount)
    {
      Entity = AppState->Entities + AppState->EntityCount++;
      
      LARGE_INTEGER Counter;
      QueryPerformanceCounter(&Counter);
      u32 Random = Counter.LowPart;
      
      u32 SeedX = (Random + 4 * (EntityIndex << 3)) << 8;
      u32 SeedY = (Random * (AppState->EntityCount << 23)) << 3;
      f32 DirX = NormalizedVectorTable[SeedX % VectorTableSize];
      f32 DirY = NormalizedVectorTable[SeedY % VectorTableSize];
      
      Entity->Pos.x =  Point.x;
      Entity->Pos.y =  Point.y;
      Entity->Pos.z =  0.0f;
      
      Entity->Dim.x = EntityDim;
      Entity->Dim.y = EntityDim; 
      Entity->Dim.z = 1.0f;
      Assert(Entity->Dim.x == 1.0f &&
             Entity->Dim.y == 1.0f);
      Entity->Acc.x = 0.0f;
      Entity->Acc.y = 0.0f;
      Entity->Acc.z = 0.0f;
      
      // NOTE(MIGUEL): Is normalize broken ?
      Entity->Vel = Normalize(V3f(DirX, DirY, 0.0f));
      Entity->Vel.z = 0.0f;
      
      Entity->Type = Entity_Moves;
      Entity->Exists = TRUE;
      
      EntityIndex++;
    }
  };
  return;
}


static void SimInit(app_state *AppState)
{
  AppState->MeterToPixels = 20.0f;
  // NOTE(MIGUEL): Sim Initialization
  AppState->EntityCount    =   10;
  AppState->EntityMaxCount = 256;
  
  // NOTE(MIGUEL): Entity Generation
  
  
  // NOTE(MIGUEL): Wall Entities
  
#if 1
#endif
  entity *EntityWallLeft   = AppState->Entities + AppState->EntityCount++;
  entity *EntityWallRight  = AppState->Entities + AppState->EntityCount++;
  entity *EntityWallBottom = AppState->Entities + AppState->EntityCount++;
  entity *EntityWallTop    = AppState->Entities + AppState->EntityCount++;
  
  entity *Entity = NULL;
  // TODO(MIGUEL): Convert from pixels to meters
  f32 CommonWidth = 2.0f;
  
  f32 SpaceWidth  = 20.0f;
  f32 SpaceHeight = 20.0f;
  
  Entity = EntityWallLeft;
  
  Entity->Dim.x = CommonWidth;
  Entity->Dim.y = SpaceHeight + (CommonWidth * 2.0f);
  Entity->Dim.z = CommonWidth;
  
  Entity->Pos.x = -1.0f * (SpaceWidth / 2.0f) - (Entity->Dim.x / 2.0f);
  Entity->Pos.y = 0.0f;
  Entity->Pos.z = 0.0f;
  Entity->Type = Entity_Wall;
  Entity->Exists = TRUE;
  
  Entity = EntityWallRight;
  Entity->Dim.x = CommonWidth;
  Entity->Dim.y = SpaceHeight + (CommonWidth * 2.0f);
  Entity->Dim.z = CommonWidth;
  
  Entity->Pos.x = 1.0f * (SpaceWidth / 2.0f) + (Entity->Dim.x / 2.0f);
  Entity->Pos.y = 0.0f;
  Entity->Pos.z = 0.0f;
  Entity->Type = Entity_Wall;
  Entity->Exists = TRUE;
  
  
  Entity = EntityWallTop;
  Entity->Dim.x = SpaceWidth + 0.0f;
  Entity->Dim.y = CommonWidth; 
  Entity->Dim.z = CommonWidth;
  
  Entity->Pos.x = 0.0f;
  Entity->Pos.y = 1.0f * (SpaceHeight / 2.0f) + (Entity->Dim.y / 2.0f);
  Entity->Pos.z = 0.0f;
  Entity->Type = Entity_Wall;
  Entity->Exists = TRUE;
  
  Entity = EntityWallBottom;
  Entity->Dim.x = SpaceWidth + 0.0f;
  Entity->Dim.y = CommonWidth; 
  Entity->Dim.z = CommonWidth;
  
  Entity->Pos.x = 0.0f;
  Entity->Pos.y = -1.0f * (SpaceHeight / 2.0f) - (Entity->Dim.y / 2.0f);
  Entity->Pos.z = 0.0f;
  Entity->Type = Entity_Wall;
  Entity->Exists = TRUE;
  CreateMovableGroup(AppState);
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
        *NearestNormalizedCollisionPoint = Max(0.0f,
                                               NormalizedCollisionPoint - T_Epsilon);
        
        Result = 1;
      }
      
    }
  }
  
  return Result;
}

void DrawRectDP(render_buffer *RenderBuffer, cam *Camera, v3f Pos, v3f Dim, v3f EulerAngles, v4f Color)
{
  RenderCmdPushQuad(RenderBuffer, Camera, Pos, Dim, EulerAngles, 0, Color);
  return;
}

void DrawRect(render_buffer *RenderBuffer, cam *Camera, i2f Rect, v3f EulerAngles, v4f Color)
{
  v3f Dim = V3f(Rect.max.x - Rect.min.x, Rect.max.y - Rect.min.y, 0.0f);
  v3f Pos = V3f(Rect.min.x + Dim.x*0.5f, Rect.min.y + Dim.y*0.5f, 0.5f);
  RenderCmdPushQuad(RenderBuffer, Camera, Pos, Dim, EulerAngles, 1, Color);
  return;
}

void
DrawPath(render_buffer *RenderBuffer, cam *Camera, v3f *PointList, u32 PointCount, v4f Color)
{
  
  RenderCmdPushLine(RenderBuffer,
                    Camera,
                    V3f(0.0f, 0.0f, 0.0f),
                    V3f(0.0f, 0.0f, 0.0f),
                    PointList, PointCount, 2.0f, 0.9f, Color);
  
  return;
}

void
DrawVector(render_buffer *RenderBuffer, cam *Camera, v3f Origin, v3f Vec, arena *LineArena)
{
  v3f *PointList = ArenaPushArray(LineArena, 2, v3f);
  
  v4f Color  = V4f(1.0f, 1.0f, 1.0f, 1.0f);
  
  PointList[0] = V3f(Origin.x + 0.0f , Origin.y + 0.0f , 0.5);
  PointList[1] = V3f(Origin.x + Vec.x, Origin.y + Vec.y, 0.5);
  
  RenderCmdPushLine(RenderBuffer,
                    Camera,
                    V3f(0.0f, 0.0f, 0.0f),
                    V3f(0.0f, 0.0f, 0.0f),
                    PointList, 2, 4.0f, 0.6f, Color);
  
  return;
}

void
DrawPerimeter(render_buffer *RenderBuffer, cam *Camera, v3f Origin, i2f Rect, arena *LineArena)
{
  v4f Color  = V4f(1.0f, 0.4f, 1.0f, 1.0f);
  u32 PointCount = 6;
  v3f *PointList = ArenaPushArray(LineArena, PointCount, v3f);
  
  PointList[0] = V3f(Origin.x + Rect.max.x, Origin.y + Rect.max.y, 0.5);
  PointList[1] = V3f(Origin.x + Rect.max.x, Origin.y + Rect.min.y, 0.5);
  PointList[2] = V3f(Origin.x + Rect.min.x, Origin.y + Rect.min.y, 0.5);
  PointList[3] = V3f(Origin.x + Rect.min.x, Origin.y + Rect.max.y, 0.5);
  PointList[4] = V3f(Origin.x + Rect.max.x, Origin.y + Rect.max.y, 0.5);
  // NOTE(MIGUEL): This would be uneccesary if i implement round line caps
  PointList[5] = V3f(Origin.x + Rect.max.x, Origin.y + Rect.min.y, 0.5);
  
  RenderCmdPushLine(RenderBuffer,
                    Camera,
                    V3f(0.0f, 0.0f, 0.0f),
                    V3f(0.0f, 0.0f, 0.0f),
                    PointList, PointCount, 3.0f, 0.2f, Color);
  return;
}

void
DrawAABB(render_buffer *RenderBuffer, cam *Camera, entity *Entity, arena *LineArena)
{
  v3f Origin = Entity->Pos;
  v3f Offset = Scale(Entity->Dim, 0.5f);
  
  v4f Color  = V4f(1.0f, 1.0f, 1.0f, 1.0f);
  u32 PointCount = 6;
  v3f *PointList = ArenaPushArray(LineArena, PointCount, v3f);
  
  PointList[0] = V3f(Origin.x + Offset.x, Origin.y + Offset.y, 0.5);
  PointList[1] = V3f(Origin.x + Offset.x, Origin.y - Offset.y, 0.5);
  PointList[2] = V3f(Origin.x - Offset.x, Origin.y - Offset.y, 0.5);
  PointList[3] = V3f(Origin.x - Offset.x, Origin.y + Offset.y, 0.5);
  PointList[4] = V3f(Origin.x + Offset.x, Origin.y + Offset.y, 0.5);
  // NOTE(MIGUEL): This would be uneccesary if i implement round line caps
  PointList[5] = V3f(Origin.x + Offset.x, Origin.y - Offset.y, 0.5);
  
  RenderCmdPushLine(RenderBuffer,
                    Camera,
                    V3f(0.0f, 0.0f, 0.0f),
                    V3f(0.0f, 0.0f, 0.0f),
                    PointList, PointCount, 10.0f, 0.8f, Color);
  return;
}

typedef struct point_cluster point_cluster;
struct point_cluster
{
  v3f *FirstPoint;
  u32 PointCount;
  arena *Arena;
  render_buffer *RenderBuffer;
};

point_cluster PointClusterInit(arena *Arena, render_buffer *RenderBuffer)
{
  point_cluster Result = {0};
  Result.FirstPoint = (v3f *)0;
  Result.PointCount  = 0;
  Result.Arena = Arena;
  Result.RenderBuffer = RenderBuffer;
  return Result;
}

void PointClusterPushPoint(point_cluster *Cluster, v3f Pos)
{
  v3f *Point = ArenaPushType(Cluster->Arena, v3f);
  if(Cluster->FirstPoint==SIM_NULL) { Cluster->FirstPoint = Point; }
  *Point = Pos;
  Cluster->PointCount++;
  return;
}

void DrawPointCluster(point_cluster *Cluster, cam *Camera)
{
  v4f  Color = V4f(1.0, 0.0, 0.0, 0.0); 
  RenderCmdPushPoints(Cluster->RenderBuffer,
                      Camera,
                      Cluster->FirstPoint,
                      Cluster->PointCount, Color);
  return;
}

void
DrawSomeText(render_buffer *RenderBuffer, str8 Text, u32 HeightInPixels, v3f Pos, v4f Color)
{
  
  // NOTE(MIGUEL): u32 HeightInPixels <- thinking of using this to size quads
#if 1
  
  u32 Line = 0;
  f32 AdvX = Pos.x;
  for (u32 Index = 0; Index < Text.Length; Index++)
  {
    u32 CharCode = (u32)Text.Data[Index];
    glyph_metrics GlyphMetrics = RenderBuffer->GlyphMetrics[CharCode];
    bitmapdata BitmapData = GlyphMetrics.BitmapData;
    switch((u8)CharCode)
    {
      case '\r':
      case '\n': 
      {
        Line++;
        AdvX = Pos.x;
      } break;
      case ' ': 
      {
        //Line++;
        //AdvX += ((u32)GlyphAdvance >> 6) * Scale;
      } break;
      default:
      {
        f32 GlyphOffsetY = (GlyphMetrics.Dim.y - GlyphMetrics.Bearing.y);
        f32 Width = GlyphMetrics.Dim.x;
        f32 Height = GlyphMetrics.Dim.y;
        f32 LineSpace = 0.0f;
        
        // NOTE(MIGUEL): Origin
        f32 OrgX = AdvX + GlyphMetrics.Bearing.x;
        f32 OrgY = Pos.y - GlyphOffsetY - (LineSpace * (f32)Line);
        
        v3f GlyphQuadPos = V3f(OrgX+Width*0.0f, OrgY+Height*0.0f, 0.5f);
        v3f GlyphQuadDim = V3f(GlyphMetrics.Dim.x*10.0f, GlyphMetrics.Dim.y*10.0f, 0.0f);
        // NOTE(MIGUEL): This is broken x should be 1.0f
        v3f CosSin = V3f(0.1, 0.0, 0.0f);
        b32 IsText = 1;
        b32 IsUI = 1;
        if(RenderBuffer->EntryCount < RenderBuffer->EntryMaxCount)
        {
          render_entry *Entry = RenderBuffer->Entries + RenderBuffer->EntryCount++;
          MemorySet(0, Entry, sizeof(render_entry));
          render_entry  RenderEntry;
          RenderEntry.Type  = RenderType_quad;
          RenderEntry.Pos   = GlyphQuadPos;
          RenderEntry.Dim   = GlyphQuadDim;
          u8 *RenderData = RenderEntry.Data;
          size_t RenderDataSize = RENDER_ENTRY_DATASEG_SIZE;
          // NOTE(MIGUEL): !!!CRITICAL!!!!This is very sensitive code. The order in which you 
          //               pop elements matters. Changing order will produce garbage data.
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &Color, sizeof(Color));
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &CosSin, sizeof(CosSin));
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &IsUI, sizeof(IsUI));
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &IsText, sizeof(IsText));
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &BitmapData, sizeof(BitmapData));
          *Entry = RenderEntry;
        }
        AdvX += GlyphMetrics.Advance;
      }
    }
  }
#endif
  return;
}

void Collision(void)
{
  u32 Iteration = 0;
  u32 MaxCollisonIterations = 4;
  for(; Iteration < MaxCollisonIterations; Iteration++)
  {
    
#if 0
    
    v3f32 *CollisionPoints = ArenaPushArray(&LineArena, MaxCollisonIterations, v3f32);
    // NOTE(MIGUEL): 
    CollisionPoints[Iteration] = V3f(Entity->Pos.x, Entity->Pos.y, 0.5);
    DrawVector(RenderBuffer, Vel, Entity->Pos, &LineArena);
    
    u32 CollisionCount = Iteration;
    v4f32 Color  = V4f(1.0f, 1.0f, 1.0f, 1.0f);
    DrawPath(RenderBuffer, CollisionPoints, CollisionCount, Color);
#endif
  }
#if 0
  if(TestEntity != Entity &&
     (TestEntity->Type == Entity_Wall ||
      TestEntity->Type == Entity_Moves) &&
     TestEntity->Exists)
  {
    Assert(Entity->Dim.x == 1.0f &&
           Entity->Dim.y == 1.0f);
    /// TEST ENTITY SPACE
    v2f EntityPos = Sub(Entity->Pos.xy, TestEntity->Pos.xy);
    f32 EntityDist = Length(V3f(Entity->Pos.x, Entity->Pos.y, 0.0f));
    if(EntityDist < ClosestEntityDist)
    {
      ClosestEntityDist = EntityDist;
      ClosestEntityId = TestEntityIndex;
    }
    
    i2f TestEntityBounds = I2fCenteredDim(TestEntity->Dim.xy);
    
    if(I2fIsInside(TestEntityBounds, EntityPos))
    {
      Entity->Exists = FALSE;
      AppState->DeadEntityCount++;
      goto ProcessNextEntity;
    }
    
    Assert(Entity->Exists == 1);
    
    i2f MinkowskiTestEntityBounds = { 0 };
    MinkowskiTestEntityBounds = R2fAddRadiusTo(TestEntityBounds,
                                               Entity->Dim.xy);
    if(Iteration==0)
    {
      v3f WeirdOffset = V3f(0.0f, 0.0f, 0.0f);
      v3f EToTEDelta = V3f(EntityPos.x, EntityPos.y, 0.5);
      DrawVector(RenderBuffer, Add(Entity->Pos,WeirdOffset), Scale(EToTEDelta, -1.0f), LineArena);
      i2f MTB = {0};
      MTB.min = Add(MinkowskiTestEntityBounds.min, WeirdOffset.xy);
      MTB.max = Add(MinkowskiTestEntityBounds.max, WeirdOffset.xy);
      DrawPerimeter(RenderBuffer, TestEntity->Pos, MTB, LineArena);
    }
    v3f WallNormal = { 0 };
    
    entity *HitEntity = NULL;
    // NOTE(MIGUEL): Intersection Test on all the walls of Test entity
    // LEFT WALL
    if(Intersects(&NormalizedCollisionPoint,
                  MinkowskiTestEntityBounds.min.x,
                  EntityPos.x, EntityPos.y,
                  PosDelta.x , PosDelta.y,
                  MinkowskiTestEntityBounds.min.y,
                  MinkowskiTestEntityBounds.max.y))
    {
      WallNormal = V3f(-1.0f, 0.0f, 0.0f);
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
      WallNormal = V3f(1.0f, 0.0f, 0.0f);
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
      WallNormal = V3f(0.0f, 1.0f, 0.0f);
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
      WallNormal = V3f(0.0f, -1.0f, 0.0f);
      HitEntity = TestEntity;
    }
    
    // NOTE(MIGUEL): Computes New Pos Delta On Collision
    if(HitEntity)
    {
      v3f NewVel = WallNormal;
      f32   VelDot = Dot(Entity->Vel, WallNormal);
      Entity->Vel  = Sub(Entity->Vel, Scale(NewVel, VelDot));
      Entity->Vel  = Sub(Entity->Vel, Scale(NewVel, VelDot));
      
      v3f NewPosDelta = WallNormal;
      f32   PosDeltaDot = Dot(PosDelta.xy, WallNormal.xy);
      PosDelta  = Sub(PosDelta, Scale(NewVel, PosDeltaDot));
      PosDelta  = Sub(PosDelta, Scale(NewVel, PosDeltaDot));
      
    }
  }
#endif
  return;
}

v3f WorldToroidalPos(i2f World, v3f Pos)
{
  //x and z may need to be changed to x and z
  v3f Result = {0};
  if     (Pos.x < World.minx) Result.x = World.maxx;
  else if(Pos.x > World.maxx) Result.y = World.minx;
  else   Result.x = Pos.x;
  if     (Pos.y < World.miny) Result.y = World.maxy;
  else if(Pos.y > World.maxy) Result.y = World.miny;
  else    Result.y = Pos.y;
  return Result;
}

v3f FlockAlign(entity *Boid, entity *Flock, u32 FlockMateCount)
{
  v3f Steer  = {0};
  v3f AvgVel = {0};
  u32 NeighborCount = 0;
  foreach(FlockMateIndex, FlockMateCount)
  {
    entity *FlockMate = Flock + FlockMateIndex;
    if(!((FlockMate->Exists == 1) && (FlockMate->Type == Entity_Moves))) continue;
    
    f32 Dist = Distance(Boid->Pos, FlockMate->Pos);
    if((Dist < 10.0f) && !IsEqual(&FlockMate, &Boid, Ref(entity)))
    {
      AvgVel = Add(AvgVel, FlockMate->Vel);
      NeighborCount++;
    }
  }
  if(NeighborCount > 0)
  {
    v3f Alignment = Scale(AvgVel, 1.0f/NeighborCount);
    Steer = Sub(Alignment, Boid->Vel);
  }
  return Steer;
}

//#include "ui.cpp"
APIPROC SIM_UPDATE(Update)
{
  //TIMED_BLOCK;
  //BEGIN_TIMED_BLOCK(Update);
  GlobalDebugState = AppState;
  
  if(AppState->IsInitialized == FALSE)
  {
    SimInit(AppState);
    AppState->IsInitialized = TRUE;
    entity   *Entity   =  AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
      if(Entity->Type == Entity_Moves)
      {
        Assert(Entity->Dim.x == 1.0f &&
               Entity->Dim.y == 1.0f);
      }
    }
  }
  
  static f32 RotZ = 0.0f;
  RotZ += 0.2f;
  
  arena *TextArena = &AppState->TextArena;
  arena *LineArena = &AppState->LineArena;
  arena  UIArena;
  arena  CameraArena;
  ArenaInit(TextArena, Kilobytes(20), Transient);
  ArenaInit(LineArena, Kilobytes(10000), (u8 *)Transient + TextArena->Size);
  ArenaInit(&UIArena , Kilobytes(10000), (u8 *)LineArena->Base + LineArena->Size);
  ArenaInit(&CameraArena, sizeof(cam)*64, (u8 *)UIArena.Base + UIArena.Size);
  
  //RenderCmdPushQuad(RenderBuffer, V3f(0.0, 0.0, 0.0f), V3f(4000.0f, 4000.0f, 1.0f), V3f(1.0f, 1.0f, 0.0f));
  str8 MsPerFrameLabel   = Str8FromArenaFormat(TextArena, "MSPerFrame: %.2f \n", AppState->DeltaTimeMS);
  str8 LongestFrameLabel = Str8FromArenaFormat(TextArena, "longestFrame: %.2f \n", AppState->LongestFrameTime);
  DrawSomeText(RenderBuffer, MsPerFrameLabel, 40, V3f(40.0f, 600.0f, 0.0f), V4f(0.0f,1.0f,1.0f,1.0f));
  DrawSomeText(RenderBuffer, LongestFrameLabel, 40, V3f(40.0f, 552.0f, 0.0f), V4f(1.0f,0.0f,1.0f,1.0f));
  
  
  f32 TurnFraction= Golden32;
  //f32 StartX = (4.0*-1.0)-0.5;
  //f32 StartY = (4.0*1.0)+0.5;
  u32 NumPoints = 300;
  
  
  cam *PointCam = CameraInit(&CameraArena, V3f(0.0, 0.0, 0.0), V3f(AppState->WindowDim.x, AppState->WindowDim.y, 0.0f), 20.0f);
  point_cluster Cluster = PointClusterInit(LineArena, RenderBuffer);
  for(u32 PointIndex = 0; PointIndex < NumPoints; PointIndex++)
  {
    // TODO(MIGUEL): Debug the point that is missing and hanging out at the origin.
    //f32 x = StartX + 1.0f * (f32)(PointIndex % 10);
    //f32 y = StartY - 1.0f * (f32)(PointIndex / 10);
    
    f32 Radius = 10.0f;
    
    f32 Dist = SqrRoot((f32)PointIndex/(NumPoints - 1.0f));
    f32 Theta = 2.0f*Pi32*TurnFraction*(f32)PointIndex;
    
    //f32 x = StartX + 1.0f * (f32)(PointIndex % 10);
    //f32 y = StartY - 1.0f * (f32)(PointIndex / 10);
    PointClusterPushPoint(&Cluster, V3f(Dist*Cosine(Theta)*Radius,
                                        Dist*Sine  (Theta)*Radius, 0.5f));
  }
  DrawPointCluster(&Cluster, PointCam);
  
  entity *FirstEntity = AppState->Entities;
  foreach(EntityIndex, AppState->EntityCount)
  {
    entity *Entity = FirstEntity + EntityIndex;
    if(Entity->Type == Entity_Moves && Entity->Exists)
    {
      
      f32 MaxSpeed = 10.1f;
      v3f Acc   = Entity->Acc;
      v3f Vel   = Entity->Vel;
      
      // BOIDS DATA
      v3f Alignment = FlockAlign(Entity, AppState->Entities, AppState->EntityCount);
      // CALC POS DELTA
      v3f WorldSpace = Scale(V3f(AppState->WindowDim.x, AppState->WindowDim.y, 0.0f), 1.0f/AppState->MeterToPixels);
      v3f PosDelta =  Add(Scale(Acc, 0.5f * Sqr((f32)AppState->DeltaTimeMS)),
                          Scale(Scale(Vel, MaxSpeed), (f32)AppState->DeltaTimeMS));
      //PosDelta = (Length(PosDelta) > WorldSpace.y)?Scale(Normalize(PosDelta), WorldSpace.y):PosDelta;
      Entity->Vel = Normalize(Add(Scale(Acc,(f32)AppState->DeltaTimeMS), Entity->Vel));
      //Entity->Acc = Normalize(Alignment);
      //APPLY MOVE
      i2f Viewport = I2fCenteredDim(WorldSpace.xy);
      //v3f NewPos   = Add(Entity->Pos, PosDelta);
      //Entity->Pos  = WorldToroidalPos(Viewport, NewPos);
      Entity->Pos  = Add(Entity->Pos, PosDelta);
    }
  }
  
  cam *EntityCam = CameraInit(&CameraArena, V3f(0.0, 0.0, 0.0), V3f(AppState->WindowDim.x, AppState->WindowDim.y, 0.0f), 20.0f);
  foreach(EntityIndex, AppState->EntityCount)
  {
    entity *Entity = FirstEntity + EntityIndex;
    if(Entity->Exists)
    {
      if(Entity->Type == Entity_Moves)
      {
        Assert(Entity->Dim.x == 1.0f &&
               Entity->Dim.y == 1.0f);
      }
      
      DrawAABB(RenderBuffer, EntityCam, Entity, LineArena);
      DrawRectDP(RenderBuffer, EntityCam, Entity->Pos,  Entity->Dim, Entity->Vel, V4f(0.0f, 0.7f,1.0f,1.0f));
    }
  }
  
  //~UI USAGE CODE
  
#if 1
#if 0
  UIStateInit(&AppState->UIState, RenderBuffer, AppState->WindowDim, AppInput, UIArena);
  ui_block NewParent = {0};
  NewParent.Rect = R2f(0.0f, 0.0f, 200.0f, 28.0f);
  NewParent.Size[Axis_Y].Value = 30.0f;
  NewParent.Color = V4f(0.0f, 0.0f, 0.0f, 1.0f);       
  UICoreParentStackPushBlock(NewParent);
  if(UIBuildButton("Open").Hover)
  {
    UIBuildButton("Save");
  }
  
  if(UIBuildButton("Export").Hover)
  {
    UIBuildBannerList("Banner");
    UIBuildBanner();
    UIBuildBanner();
  }
  
  
  UIBuildButton("No Open");
#endif
#else
  // NOTE(MIGUEL): My new interpretations of ryans ui Experimental
  // make block/widet (use parent stack to assign a parent)
  // if there is a new parent the push
  // the parent of any new block is assumed to be at the top of the stack
  // new block are responsible for checking if thety are they are parents first child and themselve if firest child
  // new block are responstible for assing themselves as the parent last child
  // what are the mechanisme for controlling weather the block gets pushed as a parent
  int KeyPointer = 0;
  if(UIBuildNewButton("Open", UIMakeKey(&KeyPointer, 0)).Hover)
  {
    UIBuildNewButton("Save", UIMakeKey(&KeyPointer, 1));
    UIBuildNewButton("Save", UIMakeKey(&KeyPointer, 2));
    UIBuildNewButton("Save", UIMakeKey(&KeyPointer, 3));
    UIBuildNewButton("Save", UIMakeKey(&KeyPointer, 4));
  }
  
#endif
  //~END UI USAGE CODE
  
  //END_TIMED_BLOCK(Update);
#if 0
  str8 EntityCountLabel = Str8FormatFromArena(TextArena, "Entity Count: %d\n",
                                              AppState->EntityCount-AppState->DeadEntityCount);
  DrawSomeText(RenderBuffer, EntityCountLabel, 20.f, V3f(10.f, 760.0f, 0.5f));
  str8 UpdateCycleLabel = Str8FormatFromArena(TextArena, "Update Cycle Count: %I64d cycles | Hits: %d hits\n",
                                              AppState->Counter[DBG_CycleCounter_Update].CycleCount,
                                              AppState->Counter[DBG_CycleCounter_Update].HitCount);
  DrawSomeText(Render Buffer, UpdateCycleLabel, 20.f, V3f(10.f, 700.0f, 0.5f));
  str8 RendererCycleLabel = Str8FormatFromArena(TextArena, "Renderer Cycle Count: %I64d cycles | Hits: %d hits\n",
                                                AppState->Counter[DBG_CycleCounter_Render].CycleCount,
                                                AppState->Counter[DBG_CycleCounter_Render].HitCount);
  AppState->Counter[DBG_CycleCounter_Update].CycleCount = 0;
  AppState->Counter[DBG_CycleCounter_Update].HitCount = 0;
  AppState->Counter[DBG_CycleCounter_Render].CycleCount = 0;
  AppState->Counter[DBG_CycleCounter_Render].HitCount = 0;
  DrawSomeText(RenderBuffer, RendererCycleLabel, 20.f, V3f(10.f, 640.0f, 0.5f));
#endif
  return;
}
