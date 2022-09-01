//-/ MACROS
#define PERMANENT_STORAGE_SIZE (Megabytes(256))
#define TRANSIENT_STORAGE_SIZE (Gigabytes(  4))


#define COBJMACROS
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

#include <timeapi.h>

#include "config.h"
#include "types.h"
#include "memory.h"
#include "string.h"
#include "os.h"
#include "math.h"

#include "renderer.h"
#include "dx11.h"
#include "app.h"
#include "assets.h"
#include "memory.c"
#include "string.c"
#include "win32_core.c" //os.c
#include "font.h"

//-/ TYPES
typedef struct buffer buffer;
struct buffer
{
  u32 Width;
  u32 Height;
  u32 BytesPerPixel;
  void *Data;
};
//-/ GLOBALS
renderer g_Renderer;
b32      g_Running  = TRUE;
b32      gPause     = FALSE;
b32      gFrameStep = TRUE;
//-/ FUNCTIONS

typedef struct plugin plugin;
struct plugin
{
  os_module   Module;
  SIM_Update *Update;
  b32         IsValid;
  datetime LastWrite;
};

#if 0
fn void win32_GetExeFileName(win32_state *State)
{
  u32 TotalLength = GetModuleFileNameA(NULL,(LPSTR)State->Buffer, sizeof(State->Buffer));
  u8 *Dir = State->Buffer;
  u8 *Exe = State->Buffer;
  for(u8 *Scan = (u8 *)State->Buffer; *Scan; ++Scan)
  {
    if(*Scan == '\\') { Exe = Scan + 1;}
  }
  u32 Length = SafeTruncateu64(CStrGetLength((char *)Exe, 0));
  State->ExeName = Str8(Exe, Length);
  State->ExeDir  = Str8(Dir, TotalLength-Length);
  return;
}
#endif
fn str8 BuildExePathFileName(os_state *State, char *FileName, arena *Arena)
{
  str8 Result = Str8Concat(State->ExeDir, Str8(FileName), Arena);
  return Result;
}
fn plugin HotLoadPlugin(str8 SourceDLLName, str8 TempDLLName, str8 LockedFileName)
{
  plugin Result = { 0 };
  if(OSFileExists(LockedFileName))
  {
    Result.LastWrite = OSFileLastWriteTime(SourceDLLName);
    CopyFile((const char *)SourceDLLName.Data,
             (const char *)TempDLLName.Data, FALSE);
    Result.Module = OSModuleLoad(TempDLLName);
    if(Result.Module)
    {
      Result.Update  = (SIM_Update *)OSModuleGetProc(Result.Module, Str8("Update"));
      Result.IsValid = (Result.Update != NULL);
    }
  }
  if(!(Result.IsValid)) { Result.Update = 0; }
  return Result;
}
fn void HotUnloadPlugin(plugin *Plugin)
{
  if(Plugin->Module) { OSModuleUnload(Plugin->Module); }
  Plugin->IsValid = FALSE;
  Plugin->Update  = 0;
  return;
}

void CircleGeometry(v3f *Buffer, u32 BufferSize, u16 *IBuffer, u32 IBufferSize, u32 Resolution,
                    u32 *VertCountResult, u32 *IndexCountResult)
{
  v3f *BufferEnd = (v3f *)((u8 *)Buffer + BufferSize);
  v3f *Vert = Buffer;
  u32 VertCount = 0;
  
  //ORIGIN
  *Vert = V3f(0.0f, 0.0f, 0.0f);
  Vert++; VertCount++;
  
  for(u32 Wedge = 0; Wedge<Resolution; Wedge++)
  {
    f32 Theta = (2.0f * Pi32 * (f32)Wedge) / (f32)Resolution;
    if((Vert + 1) < BufferEnd)
    {
      f32 CT = Cosine(Theta);
      f32 ST = Sine(Theta);
      *Vert = V3f(0.5f * CT, 0.5f * ST, 0.0f);
      Vert++; VertCount++;
    }
    else
    {
      Assert("Out of Memory!!");
    }
  }
  
  u32 IndexCount = 0;
  u16 *IBufferEnd = (u16 *)((u8 *)IBuffer + IBufferSize);
  for(u16 TriIndex = 0; TriIndex<(VertCount - 1); TriIndex++)
  {
    u16 *Tri = IBuffer + (3*TriIndex);
    if(Tri < IBufferEnd)
    {
      Tri[0] = 0;
      Tri[1] = TriIndex + 2;
      Tri[2] = TriIndex + 1;
      
      if(Tri[1] > Resolution) Tri[1] = ((TriIndex + 2)%Resolution);
      IndexCount += 3;
    }
    else
    {
      Assert("Out of Memory!!");
    }
  }
  
  if(VertCountResult ) *VertCountResult  = VertCount;
  if(IndexCountResult) *IndexCountResult = IndexCount;
  
  return;
}

void RenderPoints(renderer *Renderer,
                  app_state *AppState,
                  v3f *PointList,
                  u32 PointCount,
                  v4f Color)
{
  f32 WierdOffset = 300.0f;
  for(u32 Point=0; Point<PointCount;Point++)
  {
    Assert(PointList[Point].z == 0.5f);
    //PointList[Point] = V3f((f32)Point, (f32)Point, 0.5f);
    PointList[Point] = Scale(PointList[Point], AppState->MeterToPixels);
    PointList[Point] = Add(PointList[Point], V3f(WierdOffset, WierdOffset, 0.0f));
  }
  
  
  v3f PointVerts  [16 + 1] = {0};
  u16    PointIndeces[256] = {0};
  u32 PointIndexCount = 0;
  CircleGeometry(PointVerts, sizeof(PointVerts),
                 PointIndeces, sizeof(PointIndeces),
                 15, 0, &PointIndexCount);
  //-
  v3f LineVData[512] = { 0 };
  MemoryCopy(LineMeshVerts, sizeof(LineMeshVerts), LineVData, sizeof(LineMeshVerts));
  MemoryCopy(PointVerts, sizeof(PointVerts), LineVData+ArrayCount(LineMeshVerts), sizeof(PointVerts));
  
  D3D11_MAPPED_SUBRESOURCE InstanceMap = {0};
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->LineVInstBuffer,
                          0, D3D11_MAP_WRITE_DISCARD, 0,
                          &InstanceMap);
  MemoryCopy(PointList, sizeof(v3f)*PointCount, InstanceMap.pData, sizeof(v3f)*PointCount);
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->LineVInstBuffer, 0);
  
  D3D11_MAPPED_SUBRESOURCE Mapped = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->LineVBuffer,
                          0, D3D11_MAP_WRITE_DISCARD, 0,
                          &Mapped);
  MemoryCopy(LineVData, sizeof(LineVData), Mapped.pData, sizeof(LineVData));
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->LineVBuffer, 0);
  
  
  D3D11_MAPPED_SUBRESOURCE JIMapped = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->LineIBuffer,
                          0, D3D11_MAP_WRITE_DISCARD,
                          0, &JIMapped);
  MemoryCopy( PointIndeces, sizeof(u16)*PointIndexCount,
             JIMapped.pData, sizeof(u16)*PointIndexCount);
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->LineIBuffer, 0);
  
  //Renderer->ConstBufferHigh = {0};
  Renderer->ConstBufferHigh.Time = (f32)AppState->Time;
  Renderer->ConstBufferHigh.Color  = Color;
  Renderer->ConstBufferHigh.Width  = 10;
  Renderer->ConstBufferHigh.JoinType  = 1;
  
  D3D11_MAPPED_SUBRESOURCE MappedHigh = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0,
                          D3D11_MAP_WRITE_DISCARD,
                          0, &MappedHigh);
  MemoryCopy( &Renderer->ConstBufferHigh, sizeof(gpu_const_high), 
             MappedHigh.pData, sizeof(gpu_const_high));
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0);
  
  ID3D11Buffer *PVBuffers[2] = { Renderer->LineVBuffer, Renderer->LineVInstBuffer };
  UINT          PVStrides[2] = { sizeof(v3f), sizeof(v3f)};
  UINT          PVOffsets[2] = { sizeof(LineMeshVerts), 0};
  
  ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 2, PVBuffers, PVStrides, PVOffsets);
  ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->LineInputLayout);
  ID3D11DeviceContext_IASetIndexBuffer(Renderer->Context, Renderer->LineIBuffer, DXGI_FORMAT_R16_UINT, 0 );
  ID3D11DeviceContext_IASetPrimitiveTopology(Renderer->Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  
  ID3D11DeviceContext_VSSetShader(Renderer->Context, Renderer->LineVShader, 0, 0);
  ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
  ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
  
  ID3D11DeviceContext_PSSetShader(Renderer->Context, Renderer->LinePShader, 0, 0);
  ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
  ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
  ID3D11DeviceContext_RSSetState(Renderer->Context, Renderer->Rasterizer);
  
  ID3D11DeviceContext_DrawIndexedInstanced(Renderer->Context, PointIndexCount, PointCount, 0, 0, 0);
  
  return;
}


void DebugClearTimers(void)
{
  // NOTE(MIGUEL): print out from here??? maybe???
  for(u32 Timer=0; Timer<DBG_CycleCounter_Count; Timer++)
  {
    MemorySet(0, &GlobalDebugState[Timer], sizeof(GlobalDebugState[Timer]));
  }
  
  return;
}

void DrawLine(renderer *Renderer,
              app_state *AppState,
              cam *Camera,
              f32 LineWidth,
              v3f *PointList,
              u32 PointCount,
              v4f Color)
{
  //-ROUNDJOINS
  u32 LineCount = PointCount - 1;
  u32 JoinCount = PointCount - 2;
  for(u32 Point=0; Point<PointCount;Point++)
  {
    Assert(PointList[Point].z == 0.5f);
    PointList[Point] = Scale(PointList[Point], Camera->UnitToPixels);
    PointList[Point] = Add(PointList[Point], Scale(Camera->Dim, 0.5));
  }
  
  v3f JoinVerts  [16 + 1] = {0};
  u16    JoinIndeces[256] = {0};
  u32 JoinIndexCount = 0;
  CircleGeometry(JoinVerts, sizeof(JoinVerts),
                 JoinIndeces, sizeof(JoinIndeces),
                 15, 0, &JoinIndexCount);
  //-
  
  //-LINE VERT DATA
  v3f LineVData[512] = { 0 };
  MemoryCopy(LineMeshVerts, sizeof(LineMeshVerts), LineVData, sizeof(LineMeshVerts));
  MemoryCopy(JoinVerts, sizeof(JoinVerts), LineVData+ArrayCount(LineMeshVerts), sizeof(JoinVerts));
  
  D3D11_MAPPED_SUBRESOURCE InstanceMap = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->LineVInstBuffer,
                          0, D3D11_MAP_WRITE_DISCARD, 0,
                          &InstanceMap);
  MemoryCopy(PointList, sizeof(v3f)*PointCount, InstanceMap.pData, sizeof(v3f)*PointCount);
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->LineVInstBuffer, 0);
  
  D3D11_MAPPED_SUBRESOURCE Mapped = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->LineVBuffer,
                          0, D3D11_MAP_WRITE_DISCARD, 0,
                          &Mapped);
  MemoryCopy(LineVData, sizeof(LineVData), Mapped.pData, sizeof(LineVData));
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->LineVBuffer, 0);
  
  ID3D11Buffer *VBuffers[2] = { Renderer->LineVBuffer, Renderer->LineVInstBuffer };
  UINT          VStrides[2] = { sizeof(v3f), sizeof(v3f)};
  UINT          VOffsets[2] = { 0, 0};
  ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 2, VBuffers, VStrides, VOffsets);
  ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->LineInputLayout);
  ID3D11DeviceContext_IASetPrimitiveTopology(Renderer->Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  
  // NOTE(MIGUEL): This is only because this constant buffer isnt set to
  //               dynamic.(UpdateSubresource() call)
  // TODO(MIGUEL): Create a compiled path for a dynamic constant buffer
  
  
  Renderer->ConstBufferHigh.Time = (f32)AppState->Time;
  Renderer->ConstBufferHigh.Width  = LineWidth;
  Renderer->ConstBufferHigh.Color  = Color;
  Renderer->ConstBufferHigh.JoinType  = 0;
  
  D3D11_MAPPED_SUBRESOURCE MappedHigh = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0,
                          D3D11_MAP_WRITE_DISCARD,
                          0, &MappedHigh);
  MemoryCopy(&Renderer->ConstBufferHigh, sizeof(gpu_const_high),
             MappedHigh.pData, sizeof(gpu_const_high));
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0);
  
  ID3D11DeviceContext_VSSetShader(Renderer->Context, Renderer->LineVShader, 0, 0);
  // NOTE(MIGUEL): The first arg is register number of the buffer this means
  //               that if in the shader the CB has : register(b0) then it should
  //               be 0.
  ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
  ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
  ID3D11DeviceContext_PSSetShader(Renderer->Context, Renderer->LinePShader, 0, 0);
  ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
  ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
  ID3D11DeviceContext_DrawInstanced(Renderer->Context, ArrayCount(LineMeshVerts), LineCount, 0, 0);
  
  //-
  D3D11_MAPPED_SUBRESOURCE JIMapped = { 0 };
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->LineIBuffer,
                          0, D3D11_MAP_WRITE_DISCARD,
                          0, &JIMapped);
  MemoryCopy( JoinIndeces, sizeof(u16)*JoinIndexCount,
             JIMapped.pData, sizeof(u16)*JoinIndexCount);
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->LineIBuffer, 0);
  
  Renderer->ConstBufferHigh.Time = (f32)AppState->Time;
  Renderer->ConstBufferHigh.Color  = Color;
  Renderer->ConstBufferHigh.Width  = LineWidth;
  Renderer->ConstBufferHigh.JoinType  = 1;
  
  ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0,
                          D3D11_MAP_WRITE_DISCARD,
                          0, &MappedHigh);
  MemoryCopy( &Renderer->ConstBufferHigh, sizeof(gpu_const_high), 
             MappedHigh.pData, sizeof(gpu_const_high));
  ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0);
  
  ID3D11Buffer *JVBuffers[2] = { Renderer->LineVBuffer, Renderer->LineVInstBuffer };
  UINT          JVStrides[2] = { sizeof(v3f), sizeof(v3f)};
  UINT          JVOffsets[2] = { sizeof(LineMeshVerts), 0};
  
  ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 2, JVBuffers, JVStrides, JVOffsets);
  ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->LineInputLayout);
  ID3D11DeviceContext_IASetIndexBuffer(Renderer->Context, Renderer->LineIBuffer, DXGI_FORMAT_R16_UINT, 0 );
  ID3D11DeviceContext_IASetPrimitiveTopology(Renderer->Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  
  ID3D11DeviceContext_VSSetShader(Renderer->Context, Renderer->LineVShader, 0, 0);
  ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
  ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
  
  ID3D11DeviceContext_PSSetShader(Renderer->Context, Renderer->LinePShader, 0, 0);
  ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
  ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
  ID3D11DeviceContext_RSSetState(Renderer->Context, Renderer->Rasterizer);
  
  ID3D11DeviceContext_DrawIndexedInstanced(Renderer->Context, JoinIndexCount, JoinCount, 0, 0, 0);
  
  return;
}

void Render(renderer *Renderer, v2s WindowDim, app_state *AppState)
{
  GlobalDebugState = AppState;
  BEGIN_TIMED_BLOCK(Render);
  
  if(Renderer->Context)
  {
    if(!IsEqual(WindowDim, Renderer->WindowDim, v2s) || !AppState->IsInitialized)
    {
      m4f Proj = Orthom4f(0.0f, (f32)gState->WindowDim.x, 0.0f,
                          (f32)gState->WindowDim.y, 0.0f, 100.0f);
      
      m4f View = Viewportm4f(V2f((f32)gState->WindowDim.x,
                                 (f32)gState->WindowDim.y)); 
      Renderer->ConstBufferLow.Proj    = Proj;
      Renderer->ConstBufferLow.View    = View;
      Renderer->ConstBufferLow.Res    = V2f((f32)gState->WindowDim.x, (f32)gState->WindowDim.y);
      
      
      ID3D11DeviceContext_OMSetRenderTargets(Renderer->Context, 0, 0, 0);
      ID3D11RenderTargetView_Release(Renderer->RenderTargetView);
      IDXGISwapChain1_ResizeBuffers(Renderer->SwapChain, 0,
                                    (u32)gState->WindowDim.x,
                                    (u32)gState->WindowDim.y,
                                    DXGI_FORMAT_UNKNOWN, 0);
      
      ID3D11Texture2D* Backbuffer;
      IDXGISwapChain1_GetBuffer(Renderer->SwapChain, 0, &IID_ID3D11Texture2D, &Backbuffer);
      ID3D11Device_CreateRenderTargetView(Renderer->Device, (ID3D11Resource*)Backbuffer, NULL, &Renderer->RenderTargetView);
      ID3D11Texture2D_Release(Backbuffer);
      
      ID3D11DeviceContext_OMSetRenderTargets(Renderer->Context, 1, &Renderer->RenderTargetView, 0);
      
      D3D11_MAPPED_SUBRESOURCE MappedConst = { 0 };
      ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->CBLow, 0,
                              D3D11_MAP_WRITE_DISCARD,
                              0, &MappedConst);
      
      MemoryCopy(&Renderer->ConstBufferLow, sizeof(gpu_const_low), MappedConst.pData, sizeof(gpu_const_low));
      ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->CBLow, 0);
      Renderer->WindowDim = WindowDim;
    }
    
    ID3D11DeviceContext_OMSetRenderTargets(Renderer->Context, 1, &Renderer->RenderTargetView, 0);
    
    D3D11_VIEWPORT Viewport;
    Viewport.TopLeftY = 0;
    Viewport.TopLeftX = 0;
    Viewport.MinDepth  = 0;
    Viewport.MaxDepth  = 0;
    Viewport.Width  = (f32)gState->WindowDim.x;
    Viewport.Height = (f32)gState->WindowDim.y;
    
    ID3D11DeviceContext_RSSetViewports(Renderer->Context, 1, &Viewport );
    
    
    v4f ClearColor = V4f(0.1f, 0.1f, 0.12f, 1.0f);
    
    ID3D11DeviceContext_ClearRenderTargetView(Renderer->Context, Renderer->RenderTargetView, ClearColor.e);
    
#if 1
    render_buffer *RenderBuffer = &Renderer->RenderBuffer;
    render_entry *RenderEntry    =  Renderer->RenderBuffer.Entries;
    
    for(u32 Entry = 0; Entry < RenderBuffer->EntryCount; Entry++, RenderEntry++)
    {
      switch(RenderEntry->Type)
      {
        case RenderType_quad:
        {
#if 1
          u8 *RenderData = (u8 *)RenderEntry->Data;
          size_t BytesExtracted = 0;
          v4f Color = {0};
          v3f CosSin = {0};
          b32 IsText = {0};
          b32 IsUI = 0;
          bitmapdata BitmapData = {0};
          // NOTE(MIGUEL): !!!CRITICAL!!!!This is very sensitive code. The order in which you 
          //               pop elements matters. Changing order will produce garbage data.
          RenderCmdPopDataElm(&RenderData, &BytesExtracted, &Color, sizeof(Color));
          RenderCmdPopDataElm(&RenderData, &BytesExtracted, &CosSin, sizeof(CosSin));
          RenderCmdPopDataElm(&RenderData, &BytesExtracted, &IsUI  , sizeof(IsUI));
          RenderCmdPopDataElm(&RenderData, &BytesExtracted, &IsText, sizeof(IsText));
          RenderCmdPopDataElm(&RenderData, &BytesExtracted, &BitmapData, sizeof(BitmapData));
          b32 IsTextured = 0;
          m4f World  = {0};
          f32 WierdOffset = 300.0f;
          MemorySet(0, &Renderer->ConstBufferHigh, sizeof(Renderer->ConstBufferHigh));
          if(IsText)
          {
#if 0
            //ASSERT(0);
            ID3D11Texture2D_Release(Renderer->TextTexResource);
            ID3D11ShaderResourceView_Release(Renderer->TextTexView);
            ID3D11SamplerState_Release(Renderer->TextSamplerState);
            
            D3D11InitTextureMapping(Renderer,
                                    &Renderer->TextTexResource,
                                    DXGI_FORMAT_R8_UNORM,
                                    &Renderer->TextTexView,
                                    &Renderer->TextSamplerState,
                                    1, &BitmapData);
            IsTextured = 1;
            // NOTE(MIGUEL): High Update Frequency
            m4f T = Translatem4f(RenderEntry->Pos);
            m4f R = Rotatem4f(CosSin.x, CosSin.y);
            m4f S = Scalem4f(RenderEntry->Dim.x, RenderEntry->Dim.y, 1.0f);
            World  = Mult(T, Mult(R, S));
            ID3D11DeviceContext_PSSetShaderResources(Renderer->Context, 0, 1, &Renderer->TextTexView);
            ID3D11DeviceContext_PSSetSamplers       (Renderer->Context, 0, 1, &Renderer->TextSamplerState);
#endif
            
          }
          else
          {
            // TODO(MIGUEL): There is something up with the operator overloading of v3f's +,*,etc
            //               that disallows me the use the V3f() initializer. investigate it
            // TODO(MIGUEL): Wall verts get compressed to a point for some reason. Thats why 
            //               Wall quads arent visable. Fix it.
            v3f PixelSpacePos = Scale(RenderEntry->Pos, RenderEntry->Camera->UnitToPixels);
            v3f PixelSpaceDim = Scale(RenderEntry->Dim, RenderEntry->Camera->UnitToPixels);
            if(IsUI)
            {
              PixelSpacePos = RenderEntry->Pos;
              PixelSpaceDim = RenderEntry->Dim;
            }
            else
            {
              PixelSpacePos = Add(PixelSpacePos, Scale(RenderEntry->Camera->Dim, 0.5));
            }
            
            Renderer->ConstBufferHigh.PixelPos  = PixelSpacePos;
            // NOTE(MIGUEL): High Update Frequency
            m4f T = Translatem4f(PixelSpacePos);
            m4f R = Rotatem4f(CosSin.x, CosSin.y);
            m4f S = Scalem4f(PixelSpaceDim.x,
                             PixelSpaceDim.y, 
                             PixelSpaceDim.z);
            World = Mult(T, Mult(R, S));
          }
          // NOTE(MIGUEL): Sets the Model and How to Shade it
          u32 Stride[] = {sizeof(vertex)};
          u32 Offset[] = { 0 };
          
          // NOTE(MIGUEL): This is only because this constant buffer isnt set to
          //               dynamic.(UpdateSubresource() call)
          // TODO(MIGUEL): Create a compiled path for a dynamic constant buffer
          ID3D11DeviceContext_IASetVertexBuffers(Renderer->Context, 0, 1, &Renderer->QuadVBuffer, Stride, Offset);
          ID3D11DeviceContext_IASetInputLayout(Renderer->Context, Renderer->InputLayout);
          ID3D11DeviceContext_IASetIndexBuffer(Renderer->Context, Renderer->QuadIBuffer, DXGI_FORMAT_R16_UINT, 0 );
          ID3D11DeviceContext_IASetPrimitiveTopology(Renderer->Context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
          Renderer->ConstBufferHigh.Time   = (f32)AppState->Time;
          Renderer->ConstBufferHigh.Color   = Color;
          Renderer->ConstBufferHigh.World  = World;
          Renderer->ConstBufferHigh.IsTextured  = IsTextured;
          
          D3D11_MAPPED_SUBRESOURCE QuadVBufferMap = { 0 };
          ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->QuadVBuffer, 0,
                                  D3D11_MAP_WRITE_DISCARD,
                                  0, &QuadVBufferMap);
          MemoryCopy(QuadMeshVerts, sizeof(QuadMeshVerts), QuadVBufferMap.pData, sizeof(QuadMeshVerts));
          ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->QuadVBuffer, 0);
          
          D3D11_MAPPED_SUBRESOURCE QuadIBufferMap = { 0 };
          ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->QuadIBuffer, 0,
                                  D3D11_MAP_WRITE_DISCARD,
                                  0, &QuadIBufferMap);
          MemoryCopy(QuadMeshIndices, sizeof(QuadMeshIndices), QuadIBufferMap.pData, sizeof(QuadMeshIndices));
          ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->QuadIBuffer, 0);
          
          D3D11_MAPPED_SUBRESOURCE MappedConst = { 0 };
          ID3D11DeviceContext_Map(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0,
                                  D3D11_MAP_WRITE_DISCARD,
                                  0, &MappedConst);
          MemoryCopy(&Renderer->ConstBufferHigh, sizeof(gpu_const_high), MappedConst.pData, sizeof(gpu_const_high));
          ID3D11DeviceContext_Unmap(Renderer->Context, (ID3D11Resource *)Renderer->CBHigh, 0);
          
          ID3D11DeviceContext_VSSetShader(Renderer->Context, Renderer->VertexShader, 0, 0);
          ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
          ID3D11DeviceContext_VSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
          ID3D11DeviceContext_PSSetShader(Renderer->Context, Renderer->PixelShader , 0, 0);
          ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 0, 1, &Renderer->CBLow);
          ID3D11DeviceContext_PSSetConstantBuffers(Renderer->Context, 1, 1, &Renderer->CBHigh);
          //Renderer->Context->PSSetShaderResources(Renderer->Context0, 1, &Renderer->SmileyTexView);
          //Renderer->Context->PSSetSamplers       (Renderer->Context0, 1, &Renderer->SmileySamplerState);
          
          ID3D11DeviceContext_DrawIndexed(Renderer->Context, ArrayCount(QuadMeshIndices), 0, 0);
#endif
        }break;
        case RenderType_line:
        {
#if 1
          render_line *Line = (render_line *)RenderEntry->Data;
          
          f32 ScaledLineWidth = Line->Width * (1.0f - Line->BorderRatio);
          // TODO(MIGUEL): clamp BoarderRatio to [0, 1]
          // TODO(MIGUEL): Implement this in the shader
          //DrawLine(Renderer, AppState, Line->Width, Line->PointData, Line->PointCount,
          //v4f32Init(0.0f, 0.0f, 0.0f, 1.0f));
          DrawLine(Renderer, AppState, RenderEntry->Camera, ScaledLineWidth,
                   Line->PointData, Line->PointCount, Line->Color);
#endif
        } break;
        case RenderType_point:
        {
#if 0
          u8 *DataSegment = RenderEntry->Data;
          size_t DataSegmentReadByteCount = 0;
          render_point PointData = {0};
          RenderCmdPopDataElm(&DataSegment, &DataSegmentReadByteCount,
                              &PointData, sizeof(PointData));
          RenderPoints(Renderer, AppState,
                       PointData.PointData,
                       PointData.PointCount,
                       PointData.Color);
#endif
        } break;
        case RenderType_none:
        {
          
        } break;
      }
    }
#endif
    //~/FLIP
    IDXGISwapChain1_Present(Renderer->SwapChain, 0 , 0);
  }
  
  if(GlobalDebugState)
  {
    END_TIMED_BLOCK(Render);
    
  }
  
  return;
}


fn ID3DBlob *D3D11LoadAndCompileShader(str8 ShaderFileDir, const char *ShaderEntry,
                                       const char *ShaderTypeAndVer, const char *CallerName)
{
  ID3DBlob *ShaderBlob, *Error;
  HRESULT Status;
  u8 Buffer[Kilobytes(8)];
  arena Arena     = ArenaInit(&Arena, Kilobytes(8), &Buffer);
  arena_temp Temp = ArenaTempBegin(&Arena);
  str8 ShaderSrc  = OSFileRead(ShaderFileDir, Temp.Arena);
  ArenaTempEnd(Temp);
  UINT flags = (D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR |
                D3DCOMPILE_ENABLE_STRICTNESS        |
                D3DCOMPILE_WARNINGS_ARE_ERRORS);
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  Status = D3DCompile(ShaderSrc.Data, ShaderSrc.Length, NULL, NULL, NULL,
                      (LPCSTR)ShaderEntry, (LPCSTR)ShaderTypeAndVer, flags, 0, &ShaderBlob, &Error);
  if (FAILED(Status))
  {
    const char* message = ID3D10Blob_GetBufferPointer(Error);
    OutputDebugStringA(message);
    ConsoleLog(Arena, "[%s]: Failed to load shader of type %s !!!", ShaderTypeAndVer, CallerName);
    Assert("Failed to load shader! Look at console for details");
  }
  return ShaderBlob;
}
// NOTE(MIGUEL): SHADER STUFFFF
b32 LoadShader(renderer *Renderer,
               str8 Path,
               ID3D11VertexShader **NewVertexShader,
               ID3D11PixelShader  **NewPixelShader,
               ID3D11InputLayout  **InputLayout,
               D3D11_INPUT_ELEMENT_DESC  *ElemDesc,
               u32 ElemCount)
{
  HRESULT Status;
  /// CREATE VERTEX SHADER
  ID3DBlob *VertexShaderBuffer = D3D11LoadAndCompileShader(Path, "VS_Main", "vs_5_0", "Load Shader Function");
  Status = ID3D11Device_CreateVertexShader(Renderer->Device,
                                           ID3D10Blob_GetBufferPointer(VertexShaderBuffer),
                                           ID3D10Blob_GetBufferSize(VertexShaderBuffer),
                                           NULL, NewVertexShader);
  Assert(SUCCEEDED(Status));
  
  Status = ID3D11Device_CreateInputLayout(Renderer->Device, ElemDesc,
                                          ElemCount,
                                          ID3D10Blob_GetBufferPointer(VertexShaderBuffer),
                                          ID3D10Blob_GetBufferSize(VertexShaderBuffer),
                                          InputLayout);
  if(VertexShaderBuffer) ID3D10Blob_Release(VertexShaderBuffer);
  
  /// CREATE PIXEL SHADER
  ID3DBlob* PixelShaderBuffer = D3D11LoadAndCompileShader(Path, "PS_Main", "ps_5_0", "Load Shader Function");
  Status = ID3D11Device_CreatePixelShader(Renderer->Device,
                                          ID3D10Blob_GetBufferPointer(PixelShaderBuffer),
                                          ID3D10Blob_GetBufferSize(PixelShaderBuffer), 0,
                                          NewPixelShader);
  Assert(SUCCEEDED(Status));
  if(VertexShaderBuffer) ID3D10Blob_Release(PixelShaderBuffer);
  return TRUE; 
}

void D3D11LoadResources(renderer *Renderer, arena *AssetLoadingArena)
{
  /// LINE
  D3D11VertexBuffer(Renderer->Device, &Renderer->LineVBuffer, NULL, sizeof(v3f),  1049, 
                    Usage_Dynamic, Access_Write);
  D3D11VertexBuffer(Renderer->Device, &Renderer->LineVInstBuffer, NULL, 2*sizeof(v3f),  256, 
                    Usage_Dynamic, Access_Write);
  D3D11IndexBuffer(Renderer->Device, &Renderer->LineIBuffer, NULL, 2*sizeof(u16),  256, 
                   Usage_Dynamic, Access_Write);
  /// TRIANGLE
  D3D11VertexBuffer(Renderer->Device, &Renderer->TriangleVBuffer, TriangleMeshVerts, 
                    2*sizeof(vertex),  ArrayCount(TriangleMeshVerts), Usage_Default, Access_None);
  /// QUAD
  D3D11VertexBuffer(Renderer->Device, &Renderer->QuadVBuffer , NULL, sizeof(u8), 4095, 
                    Usage_Dynamic, Access_Write);
  D3D11IndexBuffer(Renderer->Device, &Renderer->QuadIBuffer, QuadMeshIndices, sizeof(QuadMeshIndices),  1, 
                   Usage_Dynamic, Access_Write);
  /// TEXT SQUARE
  u32 TextSpriteSize = sizeof(vertex) * ArrayCount(TextSpriteMeshVerts);
  u32 CharLimit      = 24;
  D3D11VertexBuffer(Renderer->Device, &Renderer->TextSpriteVBuffer, NULL, TextSpriteSize, CharLimit, 
                    Usage_Dynamic, Access_Write);
  D3D11IndexBuffer(Renderer->Device, &Renderer->TextSpriteIBuffer, QuadMeshIndices, 6*sizeof(u16),  CharLimit, 
                   Usage_Default, Access_None);
  //D3D11ConstantBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data,
  //u32 Size, buffer_usage Usage, Access)
  
  // NOTE(MIGUEL): SETTING GPU CONSTANTS FOR RENDERING
  StaticAssert((sizeof(gpu_const_high)%16==0), D3D11_Const_buffer_not_a_multiple_of_16);
  StaticAssert((sizeof(gpu_const_low )%16==0), D3D11_Const_buffer_not_a_multiple_of_16);
  D3D11ConstantBuffer(Renderer->Device, &Renderer->CBHigh, NULL,
                      sizeof(gpu_const_high), Usage_Dynamic, Access_Write);
  D3D11ConstantBuffer(Renderer->Device, &Renderer->CBLow, &Renderer->ConstBufferLow,
                      sizeof(gpu_const_low), Usage_Dynamic, Access_Write);
  
  gVertexLayout[0].SemanticName         = "POSITION";
  gVertexLayout[0].SemanticIndex        = 0;
  gVertexLayout[0].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
  gVertexLayout[0].InputSlot            = 0;
  gVertexLayout[0].AlignedByteOffset    = 0;
  gVertexLayout[0].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
  gVertexLayout[0].InstanceDataStepRate = 0;
  
  gVertexLayout[1].SemanticName         = "COLOR";
  gVertexLayout[1].SemanticIndex        = 0;
  gVertexLayout[1].Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
  gVertexLayout[1].InputSlot            = 0;
  gVertexLayout[1].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
  gVertexLayout[1].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
  gVertexLayout[1].InstanceDataStepRate = 0;
  
  gVertexLayout[2].SemanticName         = "TEXCOORD";
  gVertexLayout[2].SemanticIndex        = 0;
  gVertexLayout[2].Format               = DXGI_FORMAT_R32G32_FLOAT;
  gVertexLayout[2].InputSlot            = 0;
  gVertexLayout[2].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
  gVertexLayout[2].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
  gVertexLayout[2].InstanceDataStepRate = 0;
  
  ID3D11VertexShader *NewVertexShader;
  ID3D11PixelShader  *NewPixelShader;
  Renderer->ShaderPath      = Str8("..\\src\\default.hlsl");
  Renderer->ShaderLastWrite = OSFileLastWriteTime(Renderer->ShaderPath);
  Assert(LoadShader(Renderer,
                    Renderer->ShaderPath,
                    &NewVertexShader,
                    &NewPixelShader,
                    &Renderer->InputLayout,
                    gVertexLayout, gVertexLayoutCount));
  Renderer->VertexShader = NewVertexShader;
  Renderer->PixelShader  = NewPixelShader;
  
  //~Line shadeder
  
  /// SET INPUT LAYOUT
  
  
  gLineVLayout[0].SemanticName         = "POSITION";
  gLineVLayout[0].SemanticIndex        = 0;
  gLineVLayout[0].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
  gLineVLayout[0].InputSlot            = 0;
  gLineVLayout[0].AlignedByteOffset    = 0;
  gLineVLayout[0].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
  gLineVLayout[0].InstanceDataStepRate = 0;
  
  gLineVLayout[1].SemanticName         = "INST_POINT_A";
  gLineVLayout[1].SemanticIndex        = 0;
  gLineVLayout[1].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
  gLineVLayout[1].InputSlot            = 1; // NOTE(MIGUEL): This is the index into the vertexbuffer array 
  gLineVLayout[1].AlignedByteOffset    = 0;
  gLineVLayout[1].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
  gLineVLayout[1].InstanceDataStepRate = 1;
  
  gLineVLayout[2].SemanticName         = "INST_POINT_B";
  gLineVLayout[2].SemanticIndex        = 0;
  gLineVLayout[2].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
  gLineVLayout[2].InputSlot            = 1;
  gLineVLayout[2].AlignedByteOffset    = sizeof(v3f);
  gLineVLayout[2].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
  gLineVLayout[2].InstanceDataStepRate = 1;
  
  ID3D11VertexShader *NewLineVShader;
  ID3D11PixelShader  *NewLinePShader;
  Renderer->LineShaderPath  = Str8("..\\src\\lines.hlsl");
  Renderer->LineShaderLastWrite = OSFileLastWriteTime(Renderer->LineShaderPath);
  Assert(LoadShader(Renderer,
                    Renderer->LineShaderPath,
                    &NewLineVShader,
                    &NewLinePShader,
                    &Renderer->LineInputLayout,
                    gLineVLayout, gLineVLayoutCount));
  Renderer->LineVShader = NewLineVShader;
  Renderer->LinePShader = NewLinePShader;
  
  D3D11_RASTERIZER_DESC RasterizerDesc = {0};
  //RasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
  RasterizerDesc.FillMode = D3D11_FILL_SOLID;
  RasterizerDesc.CullMode = D3D11_CULL_BACK;
  RasterizerDesc.FrontCounterClockwise = 0;
  RasterizerDesc.DepthBias = 0;
  RasterizerDesc.DepthBiasClamp = 0.0f;
  RasterizerDesc.SlopeScaledDepthBias = 0.0f;
  RasterizerDesc.DepthClipEnable = 1;
  RasterizerDesc.ScissorEnable = 0;
  RasterizerDesc.MultisampleEnable = 0;
  RasterizerDesc.AntialiasedLineEnable = 0;
  
  ID3D11Device_CreateRasterizerState(Renderer->Device, &RasterizerDesc,
                                     &Renderer->Rasterizer);
  
  u32 MemBlockSize = Kilobytes(800);
  ArenaInit(&Renderer->TextureArena, MemBlockSize, 
            VirtualAlloc(0, 
                         MemBlockSize ,
                         MEM_COMMIT | MEM_RESERVE,
                         PAGE_READWRITE));
  ArenaReset(AssetLoadingArena);
  return;
}

void D3D11HotLoadShader(renderer *Renderer,
                        str8 Path,
                        datetime *LastRecordedWrite,
                        ID3D11VertexShader **VertexShader,
                        ID3D11PixelShader  **PixelShader,
                        ID3D11InputLayout  **InputLayout,
                        D3D11_INPUT_ELEMENT_DESC *ElemDesc,
                        u32 ElemDescCount,
                        arena *ShaderLoadingArena)
{
  datetime LastWrite = OSFileLastWriteTime(Path);
  if(IsEqual(LastWrite, *LastRecordedWrite, datetime)) return;
  ID3D11VertexShader *NewVertexShader;
  ID3D11PixelShader  *NewPixelShader;
  if(LoadShader(&g_Renderer, Path,
                &NewVertexShader, &NewPixelShader, InputLayout,
                ElemDesc, ElemDescCount))
  {
    ID3D11VertexShader_Release(*VertexShader);
    ID3D11PixelShader_Release (*PixelShader);
    *VertexShader = NewVertexShader;
    *PixelShader  = NewPixelShader;
    *LastRecordedWrite = LastWrite;
  }
  return;
}

void PhysicsSim(void)
{
  renderer *Renderer = &g_Renderer;
  arena AssetLoadingArena = ArenaInit(NULL, gState->TransientSize, gState->Transient);
  D3D11LoadResources(Renderer, &AssetLoadingArena); // NOTE(MIGUEL): Should this load glyphs ???
  FontInit("..\\res\\cour.ttf", 40);
  D3D11LoadTextGlyphs(Renderer, &AssetLoadingArena); Renderer->RenderBuffer.GlyphMetrics = Renderer->GlyphMetrics;
  Renderer->SmileyTex = LoadBitmapData("..\\res\\frown.bmp");
  Renderer->TextTex   = LoadBitmapData("..\\res\\text.bmp");
  D3D11InitTextureMapping(Renderer,
                          &Renderer->SmileyTexResource,
                          DXGI_FORMAT_B8G8R8A8_UNORM,
                          &Renderer->SmileyTexView,
                          &Renderer->SmileySamplerState, 1,
                          &Renderer->SmileyTex);
  D3D11InitTextureMapping(Renderer,
                          &Renderer->TextTexResource,
                          DXGI_FORMAT_R8_UNORM,
                          &Renderer->TextTexView,
                          &Renderer->TextSamplerState, 1,
                          &Renderer->TextTex);
  // NOTE(MIGUEL): Passive transformation is a transform that changes the coordinate
  //               system. (World moving around you when walking to simulate you looking around)
  //               Active transformation does not change the coordinate system instead
  //               it changes the vectors in the coordinate system. (This moving in the enviorment.)
  //               Both can be used
  str8 ModulePathSim     = BuildExePathFileName(gState, "app.dll"     , &gState->Arena);
  str8 ModulePathSimTemp = BuildExePathFileName(gState, "app_temp.dll", &gState->Arena);
  str8 ModulePathSimLock = BuildExePathFileName(gState, "lock.tmp"    , &gState->Arena);
  
  u64 TickFrequency = 0;
  u64 WorkStartTick = 0;
  u64 WorkEndTick = 0;
  u64 WorkTickDelta = 0;
  f64 MicrosElapsedWorking = 0.;
  QueryPerformanceFrequency((LARGE_INTEGER *)&TickFrequency);
  f64 TargetMicrosPerFrame = 16666.0; 
  f64 TicksToMicros = 1000000.0/(f64)TickFrequency;
  
  plugin SimCode = {0};
  // NOTE(MIGUEL): Fuck this is sloppy
  app_state *AppState = ArenaPushType(&gState->Arena, app_state);
  os_events Events = {0};
  OSEventsInit(&Events);
  while (g_Running)
  {
    // NOTE(MIGUEL): Start Timer
    
    OSEventsConsume(&Events);
    OSWindowGetNewSize();
    AppState->WindowDim = V2f((f32)gState->WindowDim.x, (f32)gState->WindowDim.y);
    if(!gPause || gFrameStep)
    {
      QueryPerformanceCounter((LARGE_INTEGER *)&WorkStartTick);
      arena ShaderLoadingArena = ArenaInit(NULL, gState->TransientSize, gState->Transient);;
      D3D11HotLoadShader(&g_Renderer,
                         g_Renderer.ShaderPath,
                         &g_Renderer.ShaderLastWrite,
                         &g_Renderer.VertexShader,
                         &g_Renderer.PixelShader,
                         &g_Renderer.InputLayout,
                         gVertexLayout,
                         gVertexLayoutCount,
                         &ShaderLoadingArena);
      D3D11HotLoadShader(&g_Renderer,
                         g_Renderer.LineShaderPath,
                         &g_Renderer.LineShaderLastWrite,
                         &g_Renderer.LineVShader,
                         &g_Renderer.LinePShader,
                         &g_Renderer.LineInputLayout,
                         gLineVLayout,
                         gLineVLayoutCount,
                         &ShaderLoadingArena);
      datetime NewWriteTime = OSFileLastWriteTime(ModulePathSim);
      {
        if(!IsEqual(NewWriteTime, SimCode.LastWrite, datetime))
        {
          HotUnloadPlugin(&SimCode);
          SimCode = HotLoadPlugin(ModulePathSim, ModulePathSimTemp, ModulePathSimLock);
        }
      }
      
      RenderBufferInit(&g_Renderer.RenderBuffer);
      
      if(SimCode.Update)
      {
        SimCode.Update(AppState, gState->Transient, gState->TransientSize, &Events, &g_Renderer.RenderBuffer);
      }
      Render(&g_Renderer, gState->WindowDim, AppState);
      
      QueryPerformanceCounter((LARGE_INTEGER *)&WorkEndTick);
      
      
      WorkTickDelta        = WorkEndTick - WorkStartTick;
      MicrosElapsedWorking = (f64)WorkTickDelta*TicksToMicros;
      
      // NOTE(MIGUEL): Idle
      u64 IdleTickDelta = 0;
      u64 IdleStartTick = WorkEndTick;
      u64 IdleEndTick = 0;
      f64 MicrosElapsedIdle = 0.0;
      
      while((MicrosElapsedWorking+MicrosElapsedIdle)<TargetMicrosPerFrame)
      {
        QueryPerformanceCounter((LARGE_INTEGER *)&IdleEndTick);
        IdleTickDelta = IdleEndTick-IdleStartTick;
        MicrosElapsedIdle = (f64)IdleTickDelta*TicksToMicros;
      }
      
      f64 FrameTimeMS = (MicrosElapsedWorking+MicrosElapsedIdle)/1000.0;
      if(FrameTimeMS>AppState->LongestFrameTime)
      {
        AppState->LongestFrameTime = FrameTimeMS;
      }
      AppState->DeltaTimeMS  = TargetMicrosPerFrame/ 1000;
      //AppState->DeltaTimeMS  = FrameTimeMS;
      AppState->DeltaTimeMS  = TargetMicrosPerFrame/1000.0;
      AppState->Time        += AppState->DeltaTimeMS;
      AppState->FrameCount++;
      gFrameStep = gFrameStep == 1?0:1;
    }
    
  }
  
  return;
}

void WinMainCRTStartup(void)
{
  OSStateInit(&gState);
  OSWindowCreate();
  OSGraphicsInit(&g_Renderer, gState->WindowDim, (HWND)gState->Window);
  
  PhysicsSim();
  
  OSGraphicsCleanup(&g_Renderer);
  OSProcessKill();
  return;
}

//~ CRT stuff
int _fltused = 0x9875;

// NOTE(MIGUEL): Clearing large Amounts of data e.g ~4gb 
//               results in a noticable slow down.
#pragma function(memset)
void *memset(void *DestInit, int Source, size_t Size)
{
  unsigned char *Dest = (unsigned char *)DestInit;
  
  while(Size--)
    *Dest++ = (unsigned char)Source;
  
  return DestInit;
}

#pragma function(memcpy)
void *memcpy(void *DestInit, void const *SourceInit, size_t Size)
{
  unsigned char *Source = (unsigned char *)SourceInit;
  unsigned char *Dest   = (unsigned char *)DestInit;
  
  while(Size--)
    *Dest++ = *Source++;
  
  return DestInit;
}

void _wassert(wchar_t const* message,
              wchar_t const* filename,
              unsigned line)
{
  return;
}
