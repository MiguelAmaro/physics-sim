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
    Assert(0);
  }
  if (FT_New_Face(FreeType, "..\\res\\cour.ttf", 0, &Face))
  {
    OutputDebugString("FreeType Error: Could not load Font");
    Assert(0);
  }
  
  AppState->MeterToPixels = 20.0f;
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
      Assert(Entity->Dim.x == 1.0f &&
             Entity->Dim.y == 1.0f);
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
  Entity   =  AppState->Entities;
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
  
  RenderCmdPushLine(RenderBuffer,
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
  
  RenderCmdPushLine(RenderBuffer,
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
  
  RenderCmdPushLine(RenderBuffer,
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
  
  RenderCmdPushLine(RenderBuffer,
                    V3f(0.0f, 0.0f, 0.0f),
                    V3f(0.0f, 0.0f, 0.0f),
                    PointList, PointCount, 10.0f, 0.8f, Color);
  return;
}

#if 1
void
DrawSomeText(render_buffer *RenderBuffer, str8 Text, u32 HeightInPixels, v3f Pos)
{
  FT_Set_Pixel_Sizes(Face, 0, HeightInPixels);
  FT_Glyph_Metrics *GlyphMetrics = &Face->glyph->metrics;
  FT_Bitmap        *GlyphBitmap  = &Face->glyph->bitmap;
  
  u32 Line = 0;
  f32 AdvX = Pos.x;
  for (u32 Index = 0; Index < Text.Length; Index++)
  {
    u32 CharCode = (u32)Text.Data[Index];
    if (FT_Load_Char(Face, CharCode, FT_LOAD_RENDER))
    {
      Assert("FreeType Error: Could not load Glyph");
      continue;
    }
    // NOTE(MIGUEL): Metrics in 26.6 Pixel Format  AdvanceX in 1/2048th vector units
    //f32 UnitConversion = 1.0f/64.0f;
    f32 UnitConversion = 1.0f/2048*100.0f;
    v2f  GlyphDim = V2f((f32)GlyphMetrics->width*UnitConversion,
                        (f32)GlyphMetrics->height*UnitConversion);
    
    v2f  GlyphBearing = V2f((f32)GlyphMetrics->horiBearingX*UnitConversion,
                            (f32)GlyphMetrics->horiBearingY* UnitConversion);
    f32  GlyphAdvance = (f32)GlyphMetrics->horiAdvance*UnitConversion;
    u8  *GlyphBitmapData = GlyphBitmap->buffer;
    u32  GlyphBitmapSize = (u32)(GlyphDim.x * GlyphDim.y);
    
    u8 Message[4096] = {0};
    stbsp_snprintf((char *)Message, 4096,
                   "Glyph Metrics | "
                   "H: %d  W: %d  | "
                   "HoriAdv: %d   | "
                   "BearingX: %d  | "
                   "Bearing Y: %d | "
                   "                "
                   "Glyph Metrics OG| "
                   "H: %d  W: %d  | "
                   "HoriAdv: %d   | "
                   "BearingX: %d  | "
                   "Bearing Y: %d | "
                   "\n",
                   (u32)GlyphDim.x, (u32)GlyphDim.y,
                   (u32)GlyphAdvance,
                   (u32)GlyphBearing.x, (u32)GlyphBearing.y,
                   GlyphMetrics->width, GlyphMetrics->height,
                   GlyphMetrics->horiAdvance,
                   GlyphMetrics->horiBearingX, GlyphMetrics->horiBearingY);
    OutputDebugString((char *)Message);
    // TODO(MIGUEL): Get a bitmap from Free type every time
    //easy and slow
    // TODO(MIGUEL): Bitmap arena. push bitmaps in and save and foward index to renderer and bind a texture
    //               for the index for each drqw call
    //Harder and more efficeient
    // TODO(MIGUEL): Figure out storage for the bitmap. atlas or whatver, max size, bitmap dim uniformity
    // TODO(MIGUEL): Figure out access in from an at atlas bitmap. instanced quads
    
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
        f32 GlyphOffsetY = (GlyphDim.y - GlyphBearing.y);
        f32 Width = GlyphDim.x;
        f32 Height = GlyphDim.y;
        u32 LineSpace = 0.0f;
        
        // NOTE(MIGUEL): Origin
        f32 OrgX = AdvX + GlyphBearing.x;
        f32 OrgY = Pos.y - GlyphOffsetY - (LineSpace * Line);
#if 0
        // TODO(MIGUEL): Use indeOrgXed Vertices instead. OnlOrgY After
        //               implementing a simple IMUI and rendering 
        //               api. Not a prioritOrgY.
        teOrgXtured_verteOrgX QuadVerts[4] =
        {
          { OrgX + Width, OrgY + Height,   1.0f, 0.0f },
          { OrgX + Width, OrgY,            1.0f, 1.0f },
          { OrgX,         OrgY,            0.0f, 1.0f },
          { OrgX,         OrgY + Height,   0.0f, 0.0f },            
        };
        u16 QuadIndices[6] = { 0, 1, 2, 0, 2, 3 };
        //OPENGL_DBG(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_ShORT, 0));
#else
        f32 vertices[6][4] =
        {
          { OrgX,         OrgY + Height,   0.0f, 0.0f },
          { OrgX,         OrgY,            0.0f, 1.0f },
          { OrgX + Width, OrgY,            1.0f, 1.0f },
          
          { OrgX,         OrgY + Height,   0.0f, 0.0f },            
          { OrgX + Width, OrgY,            1.0f, 1.0f },
          { OrgX + Width, OrgY + Height,   1.0f, 0.0f }           
        };
#endif
        v3f GlyphQuadPos = V3f(OrgX+Width*0.0f, OrgY+Width*0.1f, 0.5f);
        v3f GlyphQuadDim = V3f(GlyphDim.x*10.0f, GlyphDim.y*10.0f, 0.0f);
        v3f CosSin = V3f(0.1, 0.0f, 0.0f);
        b32 IsText = 1;
        
        bitmapdata BitmapData = {0};
        BitmapData.Width = (u32)GlyphDim.x;
        BitmapData.Height = (u32)GlyphDim.y;
        BitmapData.Pixels = (u8 *)ArenaPushArray(RenderBuffer->PixelArena, GlyphBitmapSize, u8);
        BitmapData.BytesPerPixel = sizeof(u8);
        MemoryCopy(GlyphBitmapData, GlyphBitmapSize, BitmapData.Pixels, GlyphBitmapSize);
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
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &CosSin, sizeof(CosSin));
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &IsText, sizeof(IsText));
          RenderCmdPushDataElm(&RenderData, &RenderDataSize, &BitmapData, sizeof(BitmapData));
          *Entry = RenderEntry;
        }
        AdvX += GlyphAdvance;
      }
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
  
  
  memory_arena TextArena = { 0 };
  ArenaInit(&TextArena, KILOBYTES(20), AppMemory->TransientStorage);
  memory_arena LineArena = { 0 };
  ArenaInit(&LineArena, KILOBYTES(20), (u8 *)AppMemory->TransientStorage + TextArena.Size);
  
  //RenderCmdPushRect(RenderBuffer, V3f(0.0, 0.0, 0.0f), V3f(4000.0f, 4000.0f, 1.0f), V3f(1.0f, 1.0f, 0.0f));
  str8 MsPerFrameLabel = Str8FormatFromArena(&TextArena, "MSPerFrame: %.2f \n", AppState->DeltaTimeMS);
  DrawSomeText(RenderBuffer, MsPerFrameLabel, 60, V3f(40.0f, 600.0f, 0.0f));
  entity   *Entity   =  AppState->Entities;
  for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
  {
    if(Entity->Type == Entity_Moves)
    {
      Assert(Entity->Dim.x == 1.0f &&
             Entity->Dim.y == 1.0f);
    }
    if(Entity->Type == Entity_Moves && Entity->Exists)
    {
      // NOTE(MIGUEL): Speed is Meters/Second?
      f32   Speed = 0.001f;
      v3f Drag  = {0.08f, 0.0f, 0.0f};
      v3f Acc   = Entity->Acc * 1.0f;
      v3f Vel   = Speed * (Entity->Vel);
      
      v3f PosDelta = (0.5f * Acc * Square(AppState->DeltaTimeMS) +
                      Vel * AppState->DeltaTimeMS);
      
      Entity->Vel = Acc * AppState->DeltaTimeMS + Entity->Vel;
      
      u32 MaxCollisonIterations = 4;
      f32 NormalizedCollisionPoint = 1.0f;
      f32 RemainingTravelDistance = V3fLength(PosDelta);
      u32 ClosestEntityId = UINT32_MAX;
      f32 ClosestEntityDist = 3.0f;
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
            Assert(Entity->Dim.x == 1.0f &&
                   Entity->Dim.y == 1.0f);
            /// TEST ENTITY SPACE
            v2f EntityPos = Entity->Pos.xy - TestEntity->Pos.xy;
            f32 EntityDist = V3fLength(V3f(Entity->Pos.x, Entity->Pos.y, 0.0f));
            if(EntityDist < ClosestEntityDist)
            {
              ClosestEntityDist = EntityDist;
              ClosestEntityId = TestEntityIndex;
            }
            
            r2f TestEntityBounds = R2fCenteredDim(TestEntity->Dim.xy);
            
            
            if(R2fIsInside(TestEntityBounds, EntityPos))
            {
              Entity->Exists = false;
              goto ProcessNextEntity;
            }
            
            Assert(Entity->Exists == true);
            
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
              //DrawPerimeter(RenderBuffer, TestEntity->Pos, MTB, &LineArena);
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
          
          if(ClosestEntityId != UINT32_MAX)
          {
            //v3f Dir = V3fNormalize(TestEntity[ClosestEntityId].Pos - Entity->Pos);
            //Entity->Vel = V3fLength(Entity->Vel) * V3fLerp(0.4f, V3fNormalize(Entity->Vel), Dir);
          }
        }
        
        Entity->Pos += PosDelta;
        
        
      }
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
    
    ProcessNextEntity:
    u8 blah = 1;
    
    if(Entity->Type == Entity_Moves)
    {
      Assert(Entity->Dim.x == 1.0f &&
             Entity->Dim.y == 1.0f);
    }
  }
  
  Entity =  AppState->Entities;
  for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
  {
    if(Entity->Exists)
    {
      DrawAABB(RenderBuffer, Entity, &LineArena);
      if(Entity->Type == Entity_Moves)
      {
        Assert(Entity->Dim.x == 1.0f &&
               Entity->Dim.y == 1.0f);
      }
      RenderCmdPushRect(RenderBuffer, Entity->Pos,  Entity->Dim, Entity->Vel);
    }
    
  }
  
  return;
}
