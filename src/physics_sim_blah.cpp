#include "physics_sim_blah.h"
#include "physics_sim_assets.h"
#include "physics_sim_assets.h"
#include "windows.h"

#include <ft2build.h>
#include FT_FREETYPE_H

global FT_Library FreeType;
global FT_Face    Face;

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
  if (FT_Init_FreeType(&FreeType))
  {
    OutputDebugString("FreeType Error: Could not init FreeType Library");
    ASSERT(0);
  }
  if (FT_New_Face(FreeType, "..\\res\\cour.ttf", 0, &Face))
  {
    OutputDebugString("FreeType Error: Could not load Font");
    ASSERT(0);
  }
  
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
  f32 EntityDim = 1.0f;
  u32 EntitiesWanted = 10;
  u32 VectorTableSize = ARRAY_COUNT(NormalizedVectorTable);
  
  f32 TurnFraction= GOLDEN_RATIO32;
  u32 Resolution = 1;
  f32 NumPoints = 100;
  f32 Radius = 20.0f-2.1f;
  for(u32 Index=0; Index<NumPoints; Index++)
  {
    f32 Dist = Root((f32)Index/(NumPoints - 1.0f));
    f32 Theta = 2.0f*PI32*TurnFraction*(f32)Index;
    
    v3f Point = V3f(Dist*Cosine(Theta),
                    Dist*Sine  (Theta), 0.0f); 
    
    
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
        Delta = (Radius*Point)+ -1.0*TestEntity->Pos;
        ClosetDistance = V3fLength(Delta);
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
      Entity->Dim.z = 1.0f;
      
      Entity->Acc.x = 0.0f;
      Entity->Acc.y = 0.0f;
      Entity->Acc.z = 0.0f;
      
      // NOTE(MIGUEL): Is normalize broken ?
      Entity->Vel = V3fNormalize(V3f(DirX, DirY, 0.0f));
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
DrawPath(render_buffer *RenderBuffer, v3f *PointList, u32 PointCount, v4f Color)
{
  
  PushLine(RenderBuffer,
           V3f(0.0f, 0.0f, 0.0f),
           V3f(0.0f, 0.0f, 0.0f),
           PointList, PointCount, 2.0f, 0.9f, Color);
  
  return;
}

void
DrawVector(render_buffer *RenderBuffer, v3f Origin, v3f Vec, memory_arena *LineArena)
{
  v3f *PointList = ArenaPushArray(LineArena, 2, v3f);
  
  v4f Color  = V4f(1.0f, 1.0f, 1.0f, 1.0f);
  
  PointList[0] = V3f(Origin.x + 0.0f , Origin.y + 0.0f , 0.5);
  PointList[1] = V3f(Origin.x + Vec.x, Origin.y + Vec.y, 0.5);
  
  PushLine(RenderBuffer,
           V3f(0.0f, 0.0f, 0.0f),
           V3f(0.0f, 0.0f, 0.0f),
           PointList, 2, 4.0f, 0.6f, Color);
  
  return;
}

void
DrawPerimeter(render_buffer *RenderBuffer, v3f Origin, r2f Rect, memory_arena *LineArena)
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
  
  PushLine(RenderBuffer,
           V3f(0.0f, 0.0f, 0.0f),
           V3f(0.0f, 0.0f, 0.0f),
           PointList, PointCount, 3.0f, 0.2f, Color);
  return;
}

void
DrawAABB(render_buffer *RenderBuffer, entity *Entity, memory_arena *LineArena)
{
  v3f Origin = Entity->Pos;
  v3f Offset = Entity->Dim / 2.0f;
  
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
  
  PushLine(RenderBuffer,
           V3f(0.0f, 0.0f, 0.0f),
           V3f(0.0f, 0.0f, 0.0f),
           PointList, PointCount, 10.0f, 0.8f, Color);
  return;
}
#if 0
void
DrawText(render_buffer *RenderBuffer, str8 Text, v3f Pos, memory_arena *Arena)
{
  u32 Line = 0;
  f32 AdvX = Pos.x;
  for (u32 Index = 0; Index < Text.Length; Index++)
  {
    u32 Char = Text.Data[Index];
    switch((u8)Char)
    {
      case '\n': Line++;
      AdvX = Pos.x;
    } break;
    default:
    {
      
      f32 w = Glyph.Dim.x * Scale;
      f32 h = Glyph.Dim.y * Scale;
      u32 LineSpace = 20.0;
      
      // NOTE(MIGUEL): Origin
      f32 OrgX = AdvX + Glyph.Bearing.x * Scale;
      f32 OrgY = Pos.y - (LineSpace * Line) - GlyphOffsetY;
#if 0
      // TODO(MIGUEL): Use indeOrgXed Vertices instead. OnlOrgY After
      //               implementing a simple IMUI and rendering 
      //               api. Not a prioritOrgY.
      teOrgXtured_verteOrgX QuadVerts[4] =
      {
        { OrgX + w, OrgY + h,   1.0f, 0.0f },
        { OrgX + w, OrgY,       1.0f, 1.0f },
        { OrgX,     OrgY,       0.0f, 1.0f },
        { OrgX,     OrgY + h,   0.0f, 0.0f },            
      };
      u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
      //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_ShORT, 0));
#else
      f32 vertices[6][4] =
      {
        { OrgX,     OrgY + h,   0.0f, 0.0f },
        { OrgX,     OrgY,       0.0f, 1.0f },
        { OrgX + w, OrgY,       1.0f, 1.0f },
        
        { OrgX,     OrgY + h,   0.0f, 0.0f },            
        { OrgX + w, OrgY,       1.0f, 1.0f },
        { OrgX + w, OrgY + h,   1.0f, 0.0f }           
      };
#endif
      glBindTexture(GL_TEXTURE_2D, Glyph.TexID);
      
      glBindVertexArray(OpenGL->TexturedVertAttribID);
      
      //POSITION ATTRIB
      OPENGL_DBG(glEnableVertexAttribArray(0));
      OPENGL_DBG(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(v2f), (GLvoid *)0));
      
      OPENGL_DBG(glBindBuffer(GL_ARRAY_BUFFER, OpenGL->TexturedVertBufferID));
      OPENGL_DBG((glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices)));
      
      //OPENGL_DBG(glEnable(GL_SCISSOR_TEST));
      //OPENGL_DBG(glScissor(ViewPos.x, ViewPos.y, ViewDim.x, ViewDim.y));
      OPENGL_DBG(glDrawArrays (GL_TRIANGLES, 0, 6));
      
      AdvX += (Glyph.Advance >> 6) * Scale;
    }
  }
  return;
}
#endif

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
  ArenaInit(&TextArena, KILOBYTES(3), AppMemory->TransientStorage);
  memory_arena LineArena = { 0 };
  ArenaInit(&LineArena, KILOBYTES(20), (u8 *)AppMemory->TransientStorage + TextArena.Size);
  
  
  entity   *Entity   =  AppState->Entities;
  for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
  {
    if(Entity->Type == Entity_Moves && Entity->Exists)
    {
      // NOTE(MIGUEL): Speed is Meters/Second?
      f32   Speed = 0.002f;
      v3f Drag  = {0.08f, 0.0f, 0.0f};
      v3f Acc   = Entity->Acc * 1.0f;
      v3f Vel   = Speed * (Entity->Vel);
      
      v3f PosDelta = (0.5f * Acc * Square(AppState->DeltaTimeMS) +
                      Vel * AppState->DeltaTimeMS);
      
      Entity->Vel = Acc * AppState->DeltaTimeMS + Entity->Vel;
      
      u32 MaxCollisonIterations = 4;
      f32 NormalizedCollisionPoint = 1.0f;
      f32 RemainingTravelDistance = V3fLength(PosDelta);
      
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
            v2f EntityPos = Entity->Pos.xy - TestEntity->Pos.xy;
            
            r2f TestEntityBounds = R2fCenteredDim(TestEntity->Dim.xy);
            
            
            if(R2fIsInside(TestEntityBounds, EntityPos))
            {
              Entity->Exists = false;
              goto ProcessNextEntity;
            }
            
            ASSERT(Entity->Exists == true);
            
            r2f MinkowskiTestEntityBounds = { 0 };
            MinkowskiTestEntityBounds = R2fAddRadiusTo(TestEntityBounds,
                                                       Entity->Dim.xy);
            
            if(Iteration==0)
            {
              f32 WeirdOffset = 00.0f;
              v3f EToTEDelta = V3f(EntityPos.x, EntityPos.y, 0.5);
              DrawVector(RenderBuffer,Entity->Pos + WeirdOffset, -1.0f * EToTEDelta, &LineArena);
              r2f MTB = {0};
              MTB.min = MinkowskiTestEntityBounds.min + WeirdOffset;
              MTB.max = MinkowskiTestEntityBounds.max + WeirdOffset;
              //pDrawPerimeter(RenderBuffer, TestEntity->Pos, MTB, &LineArena);
            }
            v3f WallNormal = { 0 };
            
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
            
            //RemainingTravelDistance = ;
            // NOTE(MIGUEL): Computes New Pos Delta On Collision
            if(HitEntity)
            {
              v3f NewVel = WallNormal;
              f32   VelDot = V3fInner(Entity->Vel, WallNormal);
              Entity->Vel -= NewVel * VelDot;
              Entity->Vel -= NewVel * VelDot;
              
              
              v3f NewPosDelta = WallNormal;
              f32   PosDeltaDot = V2fInner(PosDelta.xy, WallNormal.xy);
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
        
        v3f OldPos = Entity->Pos;
        //Entity->Pos.x += 300;
        //Entity->Pos.y += 300;
        //300
        Entity->Pos.x = OldPos.x;
        Entity->Pos.y = OldPos.y;
        
      }
#if 0
      
      v3f32 *CollisionPoints = ArenaPushArray(&LineArena, MaxCollisonIterations, v3f32);
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
      v3f OldPos = Entity->Pos;
      
      DrawAABB(RenderBuffer, Entity, &LineArena);
      PushRect(RenderBuffer, Entity->Pos, Entity->Dim, Entity->Vel);
      
      Entity->Pos.x = OldPos.x;
      Entity->Pos.y = OldPos.y;
    }
    
  }
  
  
  return;
}
