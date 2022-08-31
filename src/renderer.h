#ifndef RENDERER
#define RENDERER

#include <d3d11.h>
#include <dxgi1_2.h>


#include "images.h"

typedef struct render_point render_point;
struct render_point
{
  v3f *PointData;
  v4f  Color;
  u32 Offset;
  u32 PointCount;
};

typedef struct render_line render_line;
struct render_line
{
  v3f *PointData;
  v4f  Color;
  u32 Offset;
  u32 PointCount;
  f32 Width;
  f32 BorderRatio;
};

// NOTE(MIGUEL): For constant buffer by update frequency(low, high, static)


typedef struct gpu_const_high gpu_const_high;
struct gpu_const_high
{
  //Chunk[0-4] 64 Bytes
  m4f World;
  //Chunk[5] 16 Bytes
  v4f Color;
  //Chunk[6] 16 Bytes
  v3f PixelPos;
  f32 Time;
  //Chunk[7] 16 Bytes
  f32 Width;
  u32 JoinType;
  b32 IsTextured;
  u8 _padding[4];
};


typedef struct gpu_const_low gpu_const_low;
struct gpu_const_low
{
  m4f  Proj;
  m4f  View;
  v2f  Res;
  f32   _padding[2];
  
};

typedef enum render_type render_type;
enum render_type
{
  RenderType_none,
  RenderType_quad,
  RenderType_line,
  RenderType_point,
};

typedef struct cam cam;
struct cam
{
  v3f Pos;
  v3f Dim;
  f32 UnitToPixels;
};

#define RENDER_ENTRY_DATASEG_SIZE (256)
typedef struct render_entry render_entry;
struct render_entry
{
  render_type Type;
  v3f  Pos;
  v3f  Dim;
  cam *Camera;
  
  // TODO(MIGUEL): Make this into a union
  u8 Data[RENDER_ENTRY_DATASEG_SIZE];
};

typedef struct render_buffer render_buffer;
struct render_buffer
{
  render_entry  Entries[65536];
  u32           EntryCount;
  u32           EntryMaxCount;
  arena *PixelArena;
  glyph_metrics *GlyphMetrics;
  
};

typedef struct renderer renderer;
struct renderer
{
  ID3D11Device           *Device;
  ID3D11DeviceContext    *Context;
  IDXGISwapChain1        *SwapChain;
  D3D_FEATURE_LEVEL       FeatureLevel;
  ID3D11RasterizerState  *Rasterizer;
  ID3D11RenderTargetView *RenderTargetView;
  ID3D11VertexShader     *VertexShader;
  ID3D11PixelShader      *PixelShader;
  
  ID3D11VertexShader     *LineVShader;
  ID3D11PixelShader      *LinePShader;
  
  //dunno
  ID3D11Buffer           *TriangleVBuffer; // TODO(MIGUEL): should delete????
  
  //line stuff
  ID3D11InputLayout      *LineInputLayout;
  ID3D11Buffer           *LineVBuffer;
  ID3D11Buffer           *LineVInstBuffer;
  ID3D11Buffer           *LineIBuffer;
  
  //quad stuff
  ID3D11InputLayout      *InputLayout;
  ID3D11Buffer           *QuadIBuffer;
  ID3D11Buffer           *QuadVBuffer;
  
  //text stuff
  ID3D11Buffer           *TextSpriteVBuffer;
  ID3D11Buffer           *TextSpriteIBuffer;
  ID3D11Texture2D        *TextTexResource;
  ID3D11Texture2D        *SmileyTexResource;
  
  ID3D11ShaderResourceView *SmileyTexView;
  ID3D11SamplerState       *SmileySamplerState;
  bitmapdata SmileyTex;
  
  ID3D11ShaderResourceView *TextTexView;
  ID3D11SamplerState       *TextSamplerState;
  bitmapdata TextTex;
  
  render_buffer RenderBuffer;
  arena  TextureArena;
  
  ID3D11Buffer *CBHigh;
  ID3D11Buffer *CBLow;
  
  gpu_const_high   ConstBufferHigh;
  gpu_const_low    ConstBufferLow;
  
  /// For real-time shader swaping
  arena Arena;
  u8 ArenaBuffer[Kilobytes(2)];
  datetime ShaderLastWrite;
  str8     ShaderPath;
  
  glyph_metrics GlyphMetrics[4096];
  v2s WindowDim;
  /// For real-time shader swaping
  str8     LineShaderPath;
  datetime LineShaderLastWrite;
};

cam *CameraInit(arena *Arena, v3f Pos, v3f Dim, f32 UnitToPixels)
{
  cam *Result = NULL;
  Result = ArenaPushType(Arena, cam);
  Result->Pos = Pos;
  Result->Dim = Dim;
  Result->UnitToPixels = UnitToPixels;
  return Result;
}

void RenderBufferInit(render_buffer *RenderBuffer)
{
  RenderBuffer->EntryCount    = 0;
  RenderBuffer->EntryMaxCount = 65536;
  return;
}

void RenderCmdPushDataElm(u8 **RenderData, size_t *RenderDataRegionSize, void *DataElem, size_t DataElemSize)
{
  Assert(RENDER_ENTRY_DATASEG_SIZE>=*RenderDataRegionSize);
  MemoryCopy(DataElem, DataElemSize, *RenderData, *RenderDataRegionSize);
  *RenderDataRegionSize -= DataElemSize;
  *RenderData += DataElemSize;
  return;
}

void RenderCmdPopDataElm(u8 **RenderData, size_t *BytesExtracted, void *DataElem, size_t DataElemSize)
{
  size_t RenderDataRegionSize = RENDER_ENTRY_DATASEG_SIZE;
  Assert(DataElemSize<=(RenderDataRegionSize-*BytesExtracted));
  MemoryCopy(*RenderData, DataElemSize, DataElem, DataElemSize);
  *BytesExtracted += DataElemSize;
  *RenderData += DataElemSize;
  return;
}

void RenderCmdPushPoints(render_buffer *RenderBuffer, cam *Camera, v3f *PointList, u32 PointCount, v4f Color)
{
  if(RenderBuffer->EntryCount < RenderBuffer->EntryMaxCount)
  {
    render_entry *Entry = RenderBuffer->Entries + RenderBuffer->EntryCount++;
    MemorySet(0, Entry, sizeof(render_entry) );
    render_entry  RenderEntry;
    RenderEntry.Type  = RenderType_point;
    RenderEntry.Camera = Camera;
    
    render_point PointData = {0};
    PointData.PointData  = PointList;
    PointData.Offset     = 0;
    PointData.PointCount = PointCount;
    PointData.Color      = Color;
    
    u8 *   DataSegment     = RenderEntry.Data;
    size_t DataSegmentSize = RENDER_ENTRY_DATASEG_SIZE;
    RenderCmdPushDataElm(&DataSegment, &DataSegmentSize,
                         &PointData, sizeof(PointData));
    *Entry = RenderEntry;
  }
  
  return;
}

void RenderCmdPushLine(render_buffer *RenderBuffer, cam *Camera, v3f Pos, v3f Dim, v3f *PointList, u32 PointCount,
                       f32 Width, f32 BorderRatio, v4f Color)
{
  if(RenderBuffer->EntryCount < RenderBuffer->EntryMaxCount)
  {
    render_entry *Entry = RenderBuffer->Entries + RenderBuffer->EntryCount++;
    
    MemorySet(0, Entry, sizeof(render_entry) );
    
    render_entry  RenderEntry;
    StaticAssert(sizeof(render_line) <= sizeof(RenderEntry.Data),
                 render_entry_data_buffer_is_toosmall);
    
    RenderEntry.Type  = RenderType_line;
    RenderEntry.Pos   = Pos;
    RenderEntry.Dim   = Dim;
    RenderEntry.Camera = Camera;
    // TODO(MIGUEL): Add some checking
    render_line LineData = {0};
    LineData.PointData  = PointList;
    LineData.Offset     = 0;
    LineData.PointCount = PointCount;
    LineData.Color      = Color;
    LineData.Width      = Width;
    LineData.BorderRatio = BorderRatio;
    
    u8 *   DataSegment     = RenderEntry.Data;
    size_t DataSegmentSize = RENDER_ENTRY_DATASEG_SIZE;
    RenderCmdPushDataElm(&DataSegment, &DataSegmentSize,
                         &LineData, sizeof(LineData));
    *Entry = RenderEntry;
  }
  
  return;
}

void RenderCmdPushQuad(render_buffer *RenderBuffer, cam *Camera, v3f Pos, v3f Dim, v3f CosSin, b32 IsUI, v4f Color)
{
  if(RenderBuffer->EntryCount < RenderBuffer->EntryMaxCount)
  {
    render_entry *Entry = RenderBuffer->Entries + RenderBuffer->EntryCount++;
    
    MemorySet(0, Entry, sizeof(render_entry));
    
    render_entry  RenderEntry = {0};
    RenderEntry.Type  = RenderType_quad;
    RenderEntry.Pos   = Pos;
    RenderEntry.Dim   = Dim;
    RenderEntry.Camera = Camera;
    // TODO(MIGUEL): Add some checking
    u8 *RenderData = RenderEntry.Data;
    size_t RenderDataSizeRemaining = RENDER_ENTRY_DATASEG_SIZE;
    RenderCmdPushDataElm(&RenderData, &RenderDataSizeRemaining,
                         &Color, sizeof(Color));
    RenderCmdPushDataElm(&RenderData, &RenderDataSizeRemaining,
                         &CosSin, sizeof(CosSin));
    RenderCmdPushDataElm(&RenderData, &RenderDataSizeRemaining,
                         &IsUI, sizeof(IsUI));
    *Entry = RenderEntry;
  }
  
  return;
}



#endif // RENDERER