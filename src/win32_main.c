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
#include "app.h"
#include "assets.h"

#include "memory.c"
#include "string.c"
#include "win32_core.c" //os.c
#include "font.h"

/// SET INPUT LAYOUT
global D3D11_INPUT_ELEMENT_DESC gVertexLayout[3] = { 0 };
global u32 gVertexLayoutCount = 3;
global D3D11_INPUT_ELEMENT_DESC gLineVLayout[3] = { 0 };
global u32 gLineVLayoutCount = 3;

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

static void
D3D11InitTextureMapping(renderer                 *Renderer,
                        ID3D11Texture2D **Texture,
                        DXGI_FORMAT Format,
                        ID3D11ShaderResourceView **ResView,
                        ID3D11SamplerState       **SamplerState,
                        u32 SubresourceArraySize,
                        bitmapdata *BitmapData)
{
  //u32 MaxBitmapWidth; 
  //u32 MaxBitampHeigth;
  D3D11_TEXTURE2D_DESC TextDesc = { 0 };
  TextDesc.Width  = BitmapData->Width;
  TextDesc.Height = BitmapData->Height;
  TextDesc.MipLevels = 1;
  TextDesc.ArraySize = SubresourceArraySize;
  TextDesc.Format = Format;
  TextDesc.SampleDesc.Count   = 1;
  TextDesc.SampleDesc.Quality = 0;
  TextDesc.Usage     = D3D11_USAGE_DEFAULT;
  TextDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  TextDesc.CPUAccessFlags = 0;
  TextDesc.MiscFlags      = 0;
  
  D3D11_SUBRESOURCE_DATA SubresData = { 0 };
  SubresData.pSysMem     = BitmapData->Pixels;
  SubresData.SysMemPitch = BitmapData->Width * BitmapData->BytesPerPixel;
  ID3D11Device_CreateTexture2D(Renderer->Device, &TextDesc, &SubresData, Texture);
  
  D3D11_SHADER_RESOURCE_VIEW_DESC ResViewDesc = { 0 };
  ResViewDesc.Format          = TextDesc.Format;
  ResViewDesc.ViewDimension   = D3D11_SRV_DIMENSION_TEXTURE2D;
  ResViewDesc.Texture2D.MostDetailedMip = 0;
  ResViewDesc.Texture2D.MipLevels       = 1;
  ID3D11Device_CreateShaderResourceView(Renderer->Device, (ID3D11Resource*)*Texture, &ResViewDesc, ResView);
  
  D3D11_SAMPLER_DESC SamplerDesc = { 0 };
  SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  ID3D11Device_CreateSamplerState(Renderer->Device, &SamplerDesc, SamplerState);
  
  return;
}
static void
D3D11Release(renderer *Renderer)
{
  if(Renderer->RenderTargetView) ID3D11RenderTargetView_Release(Renderer->RenderTargetView);
  //if(Renderer->SwapChain       ) ID3D11SwapChain_Release();
  if(Renderer->Context         ) ID3D11DeviceContext_Release(Renderer->Context);
  if(Renderer->Device          ) ID3D11Device_Release(Renderer->Device);
  return;
}
static b32
D3D11Startup(HWND Window, renderer *Renderer)
{
  HRESULT Status;
  b32     Result = TRUE;
  
  D3D_FEATURE_LEVEL Levels[] = {D3D_FEATURE_LEVEL_11_0};
  UINT Flags = (D3D11_CREATE_DEVICE_BGRA_SUPPORT   |
                D3D11_CREATE_DEVICE_SINGLETHREADED |
                D3D11_CREATE_DEVICE_DEBUG);
  // NOTE(MIGUEL): For if I want to ceate the device and swapchain seperately.
  Status = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, Flags, Levels,
                             ARRAYSIZE(Levels), D3D11_SDK_VERSION,
                             &Renderer->Device, 0, &Renderer->Context);
  
  Assert(SUCCEEDED(Status));
  
  // NOTE(MIGUEL): The swapchain BufferCount needs to be 2 to get
  //               2 backbuffers. Change it to 1 to see the effects.
  
  DXGI_SWAP_CHAIN_DESC1 SwapChainDescription = {0};
  SwapChainDescription.BufferCount = 2; 
  SwapChainDescription.Width  = gState->WindowDim.x;
  SwapChainDescription.Height = gState->WindowDim.y;
  SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  SwapChainDescription.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SwapChainDescription.Scaling            = DXGI_SCALING_NONE;
  SwapChainDescription.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  SwapChainDescription.SampleDesc.Count   = 1;
  SwapChainDescription.SampleDesc.Quality = 0;
  
  Assert(SUCCEEDED(Status));
  {
    IDXGIFactory2* Factory;
    Status = CreateDXGIFactory(&IID_IDXGIFactory2, &Factory);
    Assert(SUCCEEDED(Status));
    Status = IDXGIFactory2_CreateSwapChainForHwnd(Factory, (IUnknown*)Renderer->Device, Window,
                                                  &SwapChainDescription, NULL, NULL, &Renderer->SwapChain);
    Assert(SUCCEEDED(Status));
    IDXGIFactory2_Release(Factory);
  }
  {
    IDXGIFactory* Factory;
    IDXGISwapChain1_GetParent(Renderer->SwapChain, &IID_IDXGIFactory, &Factory);
    IDXGIFactory_MakeWindowAssociation(Factory, Window, DXGI_MWA_NO_ALT_ENTER);
    IDXGIFactory_Release(Factory);	
  }
  ID3D11InfoQueue* Info;
  ID3D11Device_QueryInterface(Renderer->Device, &IID_ID3D11InfoQueue, &Info);
  ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
  ID3D11InfoQueue_SetBreakOnSeverity(Info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
  ID3D11InfoQueue_Release(Info);
  
#if 0
  Status = D3D11CreateDeviceAndSwapChain(0, // NOTE(MIGUEL): Get default video adapter
                                         D3D_DRIVER_TYPE_HARDWARE,
                                         0, // NOTE(MIGUEL): Handle to the dll containing the software render if one is used
                                         Flags,
                                         Levels, ARRAYSIZE(Levels),
                                         D3D11_SDK_VERSION,
                                         &SwapChainDescription,
                                         &Renderer->SwapChain,
                                         &Renderer->Device,
                                         &Renderer->FeatureLevel, &Renderer->Context );
#endif
  
  ID3D11Texture2D* Backbuffer;
  IDXGISwapChain1_GetBuffer(Renderer->SwapChain, 0, &IID_ID3D11Texture2D, &Backbuffer);
  ID3D11Device_CreateRenderTargetView(Renderer->Device, (ID3D11Resource*)Backbuffer, NULL, &Renderer->RenderTargetView);
  ID3D11Texture2D_Release(Backbuffer);
  Assert(SUCCEEDED(Status));
  
  ID3D11DeviceContext_OMSetRenderTargets(Renderer->Context, 1, &Renderer->RenderTargetView, 0);
  D3D11_VIEWPORT Viewport;
  Viewport.TopLeftX = 0.0f;
  Viewport.TopLeftY = 0.0f;
  Viewport.Width  = (f32)gState->WindowDim.x;
  Viewport.Height = (f32)gState->WindowDim.y;
  Viewport.MinDepth = 0.0f;
  Viewport.MaxDepth = 1.0f;
  ID3D11DeviceContext_RSSetViewports(Renderer->Context, 1, &Viewport);
  return Result;
}

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
  WIN32_FILE_ATTRIBUTE_DATA Ignored;
  if(!GetFileAttributesEx((const char *)LockedFileName.Data, GetFileExInfoStandard, &Ignored))
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

#if 0
#endif
/*
void
ProcessPendingMessages(app_input *Input)
{
  MSG Message = {0};
  
  while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
  {
    //PrintSystemMsg(Message.message);
    
    switch(Message.message)
    {
      case WM_QUIT:
      {
        g_Running = FALSE;
      }  break;
      
      case WM_MOUSEWHEEL:
      {
          input->mouse_wheel_delta = ((s16)(Message.wParam >> 16) / 120.0f);
          input->mouse_wheel_integral += input->mouse_wheel_delta;
      } break;
      
case WM_SYSKEYUP:
case WM_SYSKEYDOWN:
case WM_KEYDOWN:
case WM_KEYUP:
{
  u32 VKCode          = (u32)Message.wParam;
  u32 KeyWasDown      = ((Message.lParam & (1 << 30)) != 0);
  u32 KeyIsDown       = ((Message.lParam & (1 << 31)) == 0);
  u32 KeyIndex = 0;
  
  if(KeyWasDown != KeyIsDown)
  {
    if(VKCode >= 'A' && VKCode <= 'Z')
    { 
      KeyIndex = Key_a + (VKCode - 'A');
      ProcessKeyboardMessage(&Input->AlphaKeys[KeyIndex], KeyIsDown);
    }
    
    switch(VKCode)
    {
      case VK_UP    : ProcessKeyboardMessage(&Input->NavKeys[0], KeyIsDown); break;
      case VK_LEFT  : ProcessKeyboardMessage(&Input->NavKeys[1], KeyIsDown); break;
      case VK_DOWN  : ProcessKeyboardMessage(&Input->NavKeys[2], KeyIsDown); break;
      case VK_RIGHT : ProcessKeyboardMessage(&Input->NavKeys[3], KeyIsDown); break;
      case VK_ESCAPE: ProcessKeyboardMessage(&Input->NavKeys[4], KeyIsDown); break;
      case VK_SPACE : ProcessKeyboardMessage(&Input->NavKeys[5], KeyIsDown); break;
      //case VK_F4    : g_Platform.QuitApp = KeyAltWasDown ? 1 : 0; break;
    }
  }
  
  // TODO(MIGUEL): Remove This and handle else where 
  if(KeyWasDown != KeyIsDown)
  {
    switch(VKCode)
    {
      case VK_UP:
      {
      } break;
      
      case VK_LEFT:
      {
      } break;
      
      case VK_DOWN:
      {
      } break;
      
      case VK_RIGHT:
      {
        gFrameStep = TRUE;
      } break;
      
      case VK_ESCAPE:
      {
      } break;
      
      case VK_SPACE: 
      {
      } break;
      
    }
    
    if((VKCode == 'P') && (KeyIsDown))
    {
      gPause= !gPause;
    }
    
    if(KeyIsDown)
    {
      
      u32 AltKeyWasDown = ( Message.lParam & (1 << 29));
      if((VKCode == VK_F4) && AltKeyWasDown)
      {
        g_Running = FALSE;
      }
      if((VKCode == VK_RETURN) && AltKeyWasDown)
      {
        if(Message.hwnd)
        {
          //win32_toggle_fullscreen(Message.hwnd );
        }
      }
    }
  }
} break;

default:
{
  TranslateMessage(&Message);
  DispatchMessageA(&Message);
} break;
}
}

return;
}
*/

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

void
Render(renderer *Renderer, v2s WindowDim, app_state *AppState)
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

// NOTE(MIGUEL): SHADER STUFFFF
b32 LoadShader(renderer *Renderer,
               ID3D11VertexShader **NewVertexShader,
               ID3D11PixelShader  **NewPixelShader,
               ID3D11InputLayout  **InputLayout,
               D3D11_INPUT_ELEMENT_DESC  *ElemDesc,
               u32 ElemCount,
               HANDLE ShaderCodeHandle,
               size_t ShaderFileSize,
               arena *AssetLoadingArena)
{
  HRESULT Result;
  
  /// CREATE VERTEX SHADER
  ID3DBlob *VertexShaderBuffer;
  
  DWORD ShaderFlags =  D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
  
  ID3DBlob *ErrorBuffer;
  
  // NOTE(MIGUEL): D3DX11CompileFromFile is depricated
  
  u8 *ShaderCode = ArenaPushArray(AssetLoadingArena,
                                  ShaderFileSize, u8);
  
  
  ReadFile(ShaderCodeHandle, ShaderCode, SafeTruncateu64(ShaderFileSize), 0, 0);
  CloseHandle(ShaderCodeHandle);
  
  Result = D3DCompile(ShaderCode, ShaderFileSize,
                      0, 0, 0, "VS_Main", "vs_4_0", ShaderFlags, 0,
                      &VertexShaderBuffer, &ErrorBuffer);
  if(FAILED(Result))
  {
    OutputDebugString((LPCSTR)ID3D10Blob_GetBufferPointer(ErrorBuffer));
    if(ErrorBuffer != 0)
    {
      ID3D10Blob_Release(ErrorBuffer);
      return FALSE;
    }
  }
  Result = ID3D11Device_CreateVertexShader(Renderer->Device,
                                           ID3D10Blob_GetBufferPointer(VertexShaderBuffer),
                                           ID3D10Blob_GetBufferSize(VertexShaderBuffer),
                                           NULL, NewVertexShader);
  
  if(FAILED(Result))
  {
    ID3D10Blob_Release(VertexShaderBuffer);
    return FALSE;
  }
  
  Result = ID3D11Device_CreateInputLayout(Renderer->Device, ElemDesc,
                                          ElemCount,
                                          ID3D10Blob_GetBufferPointer(VertexShaderBuffer),
                                          ID3D10Blob_GetBufferSize(VertexShaderBuffer),
                                          InputLayout);
  
  if(VertexShaderBuffer) ID3D10Blob_Release(VertexShaderBuffer);
  
  /// CREATE PIXEL SHADER
  
  ID3DBlob* PixelShaderBuffer = 0;
  
  Result = D3DCompile(ShaderCode,
                      ShaderFileSize,
                      0,
                      0, 0,
                      "PS_Main",
                      "ps_4_0",
                      ShaderFlags,
                      0,
                      &PixelShaderBuffer,
                      &ErrorBuffer);
  
  if(FAILED(Result))
  {
    if(ErrorBuffer != 0)
    {
      ID3D10Blob_Release(ErrorBuffer);
      return FALSE;
    }
  }
  
  Result = ID3D11Device_CreatePixelShader(Renderer->Device,
                                          ID3D10Blob_GetBufferPointer(PixelShaderBuffer),
                                          ID3D10Blob_GetBufferSize(PixelShaderBuffer), 0,
                                          NewPixelShader);
  if(VertexShaderBuffer) ID3D10Blob_Release(PixelShaderBuffer);
  if(FAILED(Result)) { return FALSE; }
  return TRUE; 
}

void D3D11LoadResources(renderer *Renderer, arena *AssetLoadingArena)
{
  HRESULT Result;
  
  
  // NOTE(MIGUEL): !!!! DONT FORGET TO READ D3D11 OUTPUT DEBUG MESSAGES
  //                    ON FAILED ASSERTIONS FRIST!!!!!!!
  
  // NOTE(MIGUEL): MESH/MODEL STUFFFF
  {
    /// LINE
    D3D11_BUFFER_DESC LineVDesc = { 0 };
    LineVDesc.Usage = D3D11_USAGE_DYNAMIC;
    LineVDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    LineVDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    LineVDesc.ByteWidth = sizeof(v3f) * 1049;
    
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &LineVDesc,
                                       NULL,
                                       &Renderer->LineVBuffer);
    
    Assert(!FAILED(Result));
    
    D3D11_BUFFER_DESC LineVInstDesc = { 0 };
    LineVInstDesc.Usage = D3D11_USAGE_DYNAMIC;
    LineVInstDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    LineVInstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    LineVInstDesc.ByteWidth = 256 * (2 * sizeof(v3f)); //for point a & b
    
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &LineVInstDesc,
                                       NULL,
                                       &Renderer->LineVInstBuffer);
    
    Assert(!FAILED(Result));
    
    D3D11_BUFFER_DESC LineIDesc = { 0 };
    LineIDesc.Usage = D3D11_USAGE_DYNAMIC;
    LineIDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    LineIDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    LineIDesc.ByteWidth = 256 * sizeof(u16); //for point a & b
    
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &LineIDesc,
                                       NULL,
                                       &Renderer->LineIBuffer);
    
    Assert(!FAILED(Result));
    
    /// TRIANGLE
    D3D11_BUFFER_DESC TriangleVertDesc = { 0 };
    TriangleVertDesc.Usage     = D3D11_USAGE_DEFAULT;
    TriangleVertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    TriangleVertDesc.ByteWidth = sizeof(vertex) * ArrayCount(TriangleMeshVerts);
    
    D3D11_SUBRESOURCE_DATA TriangleVertData = { 0 };
    TriangleVertData.pSysMem = TriangleMeshVerts;
    
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &TriangleVertDesc,
                                       &TriangleVertData,
                                       &Renderer->TriangleVBuffer );
    Assert(!FAILED(Result));
    
    /// QUAD
    D3D11_BUFFER_DESC QuadVertDesc = { 0 };
    QuadVertDesc.Usage          = D3D11_USAGE_DYNAMIC;
    QuadVertDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    QuadVertDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    // NOTE(MIGUEL): As far as i can tell there is no reason why i cant make
    //               this arbetrarely big stream in what ever i need.
    QuadVertDesc.ByteWidth = 4095;
    /*// NOTE(MIGUEL): This is not happening because is want to stuff the
      //                 quad verts and the line verts(also quad but slightly different)
//                 in the same buffer. ill push each using updatesubresource at a 
//                 performance cost that the gpu cant just push it in and forget about it.
//                 whatever thats why im not using subresource data to init and forget
//                 and using dynamic usage.
    D3D11_SUBRESOURCE_DATA QuadVertData = { 0 };
    QuadVertData.pSysMem = 0;
    */
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &QuadVertDesc,
                                       0,
                                       &Renderer->QuadVBuffer );
    
    D3D11_BUFFER_DESC QuadIndexDesc = { 0 };
    QuadIndexDesc.Usage          = D3D11_USAGE_DYNAMIC;
    QuadIndexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    QuadIndexDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
    QuadIndexDesc.ByteWidth      = sizeof(QuadMeshIndices);
    
    D3D11_SUBRESOURCE_DATA QuadIndexData = { 0 };
    QuadIndexData.pSysMem = QuadMeshIndices;
    
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &QuadIndexDesc,
                                       &QuadIndexData,
                                       &Renderer->QuadIBuffer );
    Assert(!FAILED(Result));
    
    /// TEXT SQUARE
    
    u32 TextSpriteSize = sizeof(vertex) * ArrayCount(TextSpriteMeshVerts);
    u32 CharLimit      = 24;
    
    D3D11_BUFFER_DESC TextSpriteVertDesc = { 0 };
    TextSpriteVertDesc.Usage          = D3D11_USAGE_DYNAMIC;
    TextSpriteVertDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    TextSpriteVertDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    TextSpriteVertDesc.ByteWidth      = TextSpriteSize * CharLimit;
#if 0
    D3D11_SUBRESOURCE_DATA TextSpriteVertData = { 0 };
    TextSpriteVertData.pSysMem = nullptr;
#endif
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &TextSpriteVertDesc,
                                       0,
                                       &Renderer->TextSpriteVBuffer );
    Assert(!FAILED(Result));
    
    D3D11_BUFFER_DESC TextSpriteIndexDesc = { 0 };
    TextSpriteIndexDesc.Usage          = D3D11_USAGE_DEFAULT;
    TextSpriteIndexDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
    //TextSpriteIndexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    TextSpriteIndexDesc.ByteWidth = sizeof(u16) * (6 * CharLimit);
    
    D3D11_SUBRESOURCE_DATA TextSpriteIndexData = { 0 };
    TextSpriteIndexData.pSysMem = TextSpriteMeshIndices;
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &TextSpriteIndexDesc,
                                       &TextSpriteIndexData,
                                       &Renderer->TextSpriteIBuffer );
    Assert(!FAILED(Result));
  }
  
  
  // NOTE(MIGUEL): SETTING GPU CONSTANTS FOR RENDERING
  {
    // NOTE(MIGUEL): What is the difference between dynamic and default usage?
    //               Why cant i use updatesubresource using dynamic and how do
    //               i pass const buff data to the pipeline. 
    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/how-to--use-dynamic-resources
    D3D11_BUFFER_DESC      GPUConstantsDesc     = { 0 };
    D3D11_SUBRESOURCE_DATA GPUConstantsResource = { 0 };
    
    // NOTE(MIGUEL): This doesn't need to be pre intializeed because data
    //               is guarenteed to be set every frame before the draw call.
    GPUConstantsDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    GPUConstantsDesc.Usage          = D3D11_USAGE_DYNAMIC;
    GPUConstantsDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GPUConstantsDesc.ByteWidth      = sizeof(gpu_const_high);
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &GPUConstantsDesc,
                                       0,
                                       &Renderer->CBHigh);
    StaticAssert((sizeof(gpu_const_high)%16==0), D3D11_Const_buffer_not_a_multiple_of_16);
    Assert(!FAILED(Result));
    
    GPUConstantsDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    GPUConstantsDesc.Usage          = D3D11_USAGE_DYNAMIC;
    GPUConstantsDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    GPUConstantsDesc.ByteWidth      = sizeof(gpu_const_low);
    GPUConstantsResource.pSysMem = &Renderer->ConstBufferLow;
    
    Result = ID3D11Device_CreateBuffer(Renderer->Device, &GPUConstantsDesc,
                                       &GPUConstantsResource,
                                       &Renderer->CBLow);
    Assert(!FAILED(Result));
  }
  
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
  
  WIN32_FIND_DATAA *CurrentShaderFileInfo = &Renderer->CurrentShaderFileInfo;
  
  memcpy(&Renderer->CurrentShaderPath,
         "..\\src\\default.hlsl",
         ArrayCount("..\\src\\default.hlsl"));
  
  char *ShaderPath = Renderer->CurrentShaderPath;
  
  
  FindFirstFileA(ShaderPath,
                 CurrentShaderFileInfo);
  
  HANDLE *ShaderCodeHandle = &Renderer->InUseShaderFileA;
  
  *ShaderCodeHandle = CreateFileA(ShaderPath,
                                  GENERIC_READ, 0, 0,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  0);
  
  Assert(*ShaderCodeHandle);
  
  // NOTE(MIGUEL): Use an arena
  
  size_t ShaderFileSize = ((CurrentShaderFileInfo->nFileSizeHigh << 31) |
                           (CurrentShaderFileInfo->nFileSizeLow));
  
  ID3D11VertexShader *NewVertexShader;
  ID3D11PixelShader  *NewPixelShader;
  Assert(LoadShader(Renderer,
                    &NewVertexShader,
                    &NewPixelShader,
                    &Renderer->InputLayout,
                    gVertexLayout, gVertexLayoutCount,
                    *ShaderCodeHandle,
                    ShaderFileSize,
                    AssetLoadingArena));
  
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
  
  
  WIN32_FIND_DATAA *LineShaderFileInfo = &Renderer->LineShaderFileInfo;
  
  memcpy(&Renderer->LineShaderPath,
         "..\\src\\lines.hlsl",
         ArrayCount("..\\src\\lines.hlsl"));
  
  char *LineShaderPath = Renderer->LineShaderPath;
  
  
  FindFirstFileA(LineShaderPath,
                 LineShaderFileInfo);
  
  HANDLE *LineShaderCodeHandle = &Renderer->InUseLineShaderFileA;
  
  *LineShaderCodeHandle = CreateFileA(LineShaderPath,
                                      GENERIC_READ, 0, 0,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL,
                                      0);
  
  Assert(*LineShaderCodeHandle);
  
  size_t LineShaderFileSize = ((LineShaderFileInfo->nFileSizeHigh << 31) |
                               (LineShaderFileInfo->nFileSizeLow));
  
  ID3D11VertexShader *NewLineVShader;
  ID3D11PixelShader  *NewLinePShader;
  Assert(LoadShader(Renderer,
                    &NewLineVShader,
                    &NewLinePShader,
                    &Renderer->LineInputLayout,
                    gLineVLayout, gLineVLayoutCount,
                    *LineShaderCodeHandle,
                    LineShaderFileSize,
                    AssetLoadingArena));
  
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
                        ID3D11InputLayout  **InputLayout,
                        D3D11_INPUT_ELEMENT_DESC *ElemDesc,
                        u32 ElemDescCount,
                        arena *ShaderLoadingArena)
{
  WIN32_FIND_DATAA *CurrentShaderFileInfo = &Renderer->CurrentShaderFileInfo;
  WIN32_FIND_DATAA  UpdatedShaderFileInfo = {0};
  
  //char *CurrentShaderPath = Renderer->CurrentShaderPath;
  FindFirstFileA("..\\src\\default.hlsl",
                 &UpdatedShaderFileInfo);
  
  
  if((UpdatedShaderFileInfo.ftLastWriteTime.dwLowDateTime !=
      CurrentShaderFileInfo->ftLastWriteTime.dwLowDateTime) ||
     (UpdatedShaderFileInfo.ftLastWriteTime.dwHighDateTime !=
      CurrentShaderFileInfo->ftLastWriteTime.dwHighDateTime)) 
  {
    
    ID3D11VertexShader *NewVertexShader = NULL;
    ID3D11PixelShader  *NewPixelShader  = NULL;
    
    if(Renderer->InUseShaderFileA)
    {
      
      char PostFix[] = "_inuse_b.hlsl";
      
      CopyFile("..\\src\\default.hlsl",
               "..\\src\\default""_inuse_b"".hlsl", 0);
      
      
      size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 31) |
                               (UpdatedShaderFileInfo.nFileSizeLow));
      
      Renderer->InUseShaderFileB = CreateFileA("..\\src\\default""_inuse_b"".hlsl",
                                               GENERIC_READ, 0, 0,
                                               OPEN_EXISTING,
                                               FILE_FLAG_DELETE_ON_CLOSE,
                                               0);
      
      if(LoadShader(&g_Renderer,
                    &NewVertexShader,
                    &NewPixelShader,
                    InputLayout,
                    ElemDesc, ElemDescCount,
                    Renderer->InUseShaderFileB,
                    ShaderFileSize,
                    ShaderLoadingArena))
      {
        ID3D11VertexShader_Release(Renderer->VertexShader);
        ID3D11PixelShader_Release(Renderer->PixelShader);
        
        Renderer->VertexShader = NewVertexShader;
        Renderer->PixelShader  = NewPixelShader;
        
      }
      
      CloseHandle(Renderer->InUseShaderFileA);
      Renderer->InUseShaderFileA = 0;
      
      CurrentShaderFileInfo->ftLastWriteTime =
        UpdatedShaderFileInfo.ftLastWriteTime;
    }
    else if(Renderer->InUseShaderFileB)
    {
      
      CopyFile("..\\src\\default"".hlsl",
               "..\\src\\default""_inuse_a"".hlsl", 0);
      
      
      size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 31) |
                               (UpdatedShaderFileInfo.nFileSizeLow));
      
      Renderer->InUseShaderFileA = CreateFileA("..\\src\\default""_inuse_a"".hlsl",
                                               GENERIC_READ, 0, 0,
                                               OPEN_EXISTING,
                                               FILE_FLAG_DELETE_ON_CLOSE,
                                               0);
      
      if(LoadShader(&g_Renderer,
                    &NewVertexShader,
                    &NewPixelShader,
                    InputLayout,
                    ElemDesc, ElemDescCount,
                    Renderer->InUseShaderFileA,
                    ShaderFileSize,
                    ShaderLoadingArena))
      {
        
        ID3D11VertexShader_Release(Renderer->VertexShader);
        ID3D11PixelShader_Release(Renderer->PixelShader);
        Renderer->VertexShader = NewVertexShader;
        Renderer->PixelShader  = NewPixelShader;
        
      }
      
      CloseHandle(Renderer->InUseShaderFileB);
      Renderer->InUseShaderFileB = 0;
      
      CurrentShaderFileInfo->ftLastWriteTime = 
        UpdatedShaderFileInfo.ftLastWriteTime;
    }
  }
  
  return;
}

void D3D11HotLoadShaderLines(renderer *Renderer,
                             ID3D11InputLayout  **InputLayout,
                             D3D11_INPUT_ELEMENT_DESC *ElemDesc,
                             u32 ElemDescCount,
                             arena *ShaderLoadingArena)
{
  WIN32_FIND_DATAA *CurrentShaderFileInfo = &Renderer->CurrentShaderFileInfo;
  WIN32_FIND_DATAA  UpdatedShaderFileInfo = {0};
  
  //char *CurrentShaderPath = Renderer->CurrentShaderPath;
  FindFirstFileA("..\\src\\lines.hlsl",
                 &UpdatedShaderFileInfo);
  
  
  if((UpdatedShaderFileInfo.ftLastWriteTime.dwLowDateTime !=
      CurrentShaderFileInfo->ftLastWriteTime.dwLowDateTime) ||
     (UpdatedShaderFileInfo.ftLastWriteTime.dwHighDateTime !=
      CurrentShaderFileInfo->ftLastWriteTime.dwHighDateTime)) 
  {
    
    ID3D11VertexShader *NewVertexShader = NULL;
    ID3D11PixelShader  *NewPixelShader  = NULL;
    
    if(Renderer->InUseShaderFileA)
    {
      
      char PostFix[] = "_inuse_b.hlsl";
      
      CopyFile("..\\src\\lines.hlsl",
               "..\\src\\lines""_inuse_b"".hlsl", 0);
      
      
      size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 31) |
                               (UpdatedShaderFileInfo.nFileSizeLow));
      
      Renderer->InUseShaderFileB = CreateFileA("..\\src\\lines""_inuse_b"".hlsl",
                                               GENERIC_READ, 0, 0,
                                               OPEN_EXISTING,
                                               FILE_FLAG_DELETE_ON_CLOSE,
                                               0);
      
      if(LoadShader(&g_Renderer,
                    &NewVertexShader,
                    &NewPixelShader,
                    InputLayout,
                    ElemDesc, ElemDescCount,
                    Renderer->InUseShaderFileB,
                    ShaderFileSize,
                    ShaderLoadingArena))
      {
        ID3D11VertexShader_Release(Renderer->LineVShader);
        ID3D11PixelShader_Release(Renderer->LinePShader);
        Renderer->LineVShader = NewVertexShader;
        Renderer->LinePShader  = NewPixelShader;
        
      }
      
      CloseHandle(Renderer->InUseShaderFileA);
      Renderer->InUseShaderFileA = 0;
      
      CurrentShaderFileInfo->ftLastWriteTime =
        UpdatedShaderFileInfo.ftLastWriteTime;
    }
    else if(Renderer->InUseShaderFileB)
    {
      
      CopyFile("..\\src\\lines"".hlsl",
               "..\\src\\lines""_inuse_a"".hlsl", 0);
      
      
      size_t ShaderFileSize = ((UpdatedShaderFileInfo.nFileSizeHigh << 31) |
                               (UpdatedShaderFileInfo.nFileSizeLow));
      
      Renderer->InUseShaderFileA = CreateFileA("..\\src\\lines""_inuse_a"".hlsl",
                                               GENERIC_READ, 0, 0,
                                               OPEN_EXISTING,
                                               FILE_FLAG_DELETE_ON_CLOSE,
                                               0);
      
      if(LoadShader(&g_Renderer,
                    &NewVertexShader,
                    &NewPixelShader,
                    InputLayout,
                    ElemDesc, ElemDescCount,
                    Renderer->InUseShaderFileA,
                    ShaderFileSize,
                    ShaderLoadingArena))
      {
        
        ID3D11VertexShader_Release(Renderer->LineVShader);
        ID3D11PixelShader_Release(Renderer->LinePShader);
        
        
        Renderer->LineVShader = NewVertexShader;
        Renderer->LinePShader  = NewPixelShader;
        
      }
      
      CloseHandle(Renderer->InUseShaderFileB);
      Renderer->InUseShaderFileB = 0;
      
      CurrentShaderFileInfo->ftLastWriteTime = 
        UpdatedShaderFileInfo.ftLastWriteTime;
    }
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
    AppState->WindowDim = V2f((f32)gState->WindowDim.x, (f32)gState->WindowDim.y);
    
    OSEventsConsume(&Events);
    OSWindowGetNewSize();
    if(!gPause || gFrameStep)
    {
      QueryPerformanceCounter((LARGE_INTEGER *)&WorkStartTick);
      arena ShaderLoadingArena = ArenaInit(NULL, gState->TransientSize, gState->Transient);;
      D3D11HotLoadShader(&g_Renderer,
                         &g_Renderer.InputLayout,
                         gVertexLayout,
                         gVertexLayoutCount,
                         &ShaderLoadingArena);
      D3D11HotLoadShaderLines(&g_Renderer,
                              &g_Renderer.LineInputLayout,
                              gLineVLayout,
                              gLineVLayoutCount,
                              &ShaderLoadingArena);
      datetime NewWriteTime = OSFileLastWriteTime(ModulePathSim);
      {
        if(!IsEqual(NewWriteTime, SimCode.LastWrite, datetime))
        {
          HotUnloadPlugin(&SimCode);
          SimCode = HotLoadPlugin(ModulePathSim,
                                  ModulePathSimTemp,
                                  ModulePathSimLock);
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
  Assert(D3D11Startup((HWND)gState->Window, &g_Renderer));
  PhysicsSim();
  D3D11Release(&g_Renderer);
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
