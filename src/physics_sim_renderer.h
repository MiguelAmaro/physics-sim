#ifndef PHYSICS_SIM_RENDERER
#define PHYSICS_SIM_RENDERER

#include <d3d11.h>
#include "physics_sim_math.h"
#include "physics_sim_types.h"

struct bitmapdata
{
  u32  Width;
  u32  Height;
  u8  *Pixels;
  u32  BytesPerPixel;
};

#pragma pack(push, 1)
struct bitmapheader
{
  u16 FileType;
  u32 FileSize;
  u16 reserved_1;
  u16 reserved_2;
  u32 BitmapOffset;
  u32 Size;
  s32 Width;
  s32 Height;
  u16 Planes;
  u16 BitsPerPixel;
  u32 Compression;
  u32 BitmapSize;
  s32 HRes;
  s32 VRes;
  u32 ColorsUsed;
  u32 ColorsImportant;
  
  u32 RedMask;
  u32 GreenMask;
  u32 BlueMask;
};
#pragma pack(pop)


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
struct gpu_const_high
{
  m4f World;
  v4f Color;
  f32 Time;
  f32 Width;
  u32 JoinType;
  v3f PixelPos;
  b32 IsTextured;
  f32 _padding[17];
};

struct gpu_const_low
{
  m4f  Proj;
  m4f  View;
  v2f  Res;
  f32   _padding[2];
  
};

enum render_type
{
  RenderType_tri,
  RenderType_quad,
  RenderType_line,
};

#define RENDER_ENTRY_DATASEG_SIZE (256)
struct render_entry
{
  render_type Type;
  v3f Pos;
  v3f Dim;
  
  u8 Data[RENDER_ENTRY_DATASEG_SIZE];
};

struct render_buffer
{
  render_entry  Entries[65536];
  u32           EntryCount;
  u32           EntryMaxCount;
  memory_arena *PixelArena;
};

struct renderer
{
  ID3D11Device           *Device;
  ID3D11DeviceContext    *Context;
  IDXGISwapChain         *SwapChain;
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
  
  ID3D11Buffer *CBHigh;
  ID3D11Buffer *CBLow;
  
  gpu_const_high   ConstBufferHigh;
  gpu_const_low    ConstBufferLow;
  
  /// For real-time shader swaping
  WIN32_FIND_DATAA CurrentShaderFileInfo;
  char             CurrentShaderPath[MAX_PATH];
  HANDLE InUseShaderFileA;
  HANDLE InUseShaderFileB;
  
  memory_arena PixelArena;
  
  /// For real-time shader swaping
  WIN32_FIND_DATAA LineShaderFileInfo;
  char             LineShaderPath[MAX_PATH];
  HANDLE InUseLineShaderFileA;
  HANDLE InUseLineShaderFileB;
};

void RenderBufferInit(render_buffer *RenderBuffer)
{
  RenderBuffer->EntryCount    = 0;
  RenderBuffer->EntryMaxCount = 65536;
  ArenaReset(RenderBuffer->PixelArena);
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

void RenderCmdPushLine(render_buffer *RenderBuffer, v3f Pos, v3f Dim, v3f *PointList, u32 PointCount,
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
    // TODO(MIGUEL): Add some checking
    render_line *LineData = (render_line *)&RenderEntry.Data;
    LineData->PointData  = PointList;
    LineData->Offset     = 0;
    LineData->PointCount = PointCount;
    LineData->Color      = Color;
    LineData->Width      = Width;
    LineData->BorderRatio = BorderRatio;
    
    *Entry = RenderEntry;
  }
  
  return;
}

void RenderCmdPushRect(render_buffer *RenderBuffer, v3f Pos, v3f Dim, v3f CosSin)
{
  if(RenderBuffer->EntryCount < RenderBuffer->EntryMaxCount)
  {
    render_entry *Entry = RenderBuffer->Entries + RenderBuffer->EntryCount++;
    
    MemorySet(0, Entry, sizeof(render_entry));
    
    render_entry  RenderEntry;
    RenderEntry.Type  = RenderType_quad;
    RenderEntry.Pos   = Pos;
    RenderEntry.Dim   = Dim;
    // TODO(MIGUEL): Add some checking
    u8 *RenderData = RenderEntry.Data;
    size_t RenderDataSizeRemaining = RENDER_ENTRY_DATASEG_SIZE;
    RenderCmdPushDataElm(&RenderData, &RenderDataSizeRemaining,
                         &CosSin, sizeof(CosSin));
    
    *Entry = RenderEntry;
  }
  
  return;
}

#endif // PHYSICS_SIM_RENDERER