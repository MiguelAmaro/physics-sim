#ifndef DX11_H
#define DX11_H

/// SET INPUT LAYOUT
global D3D11_INPUT_ELEMENT_DESC gVertexLayout[3] = { 0 };
global u32 gVertexLayoutCount = 3;
global D3D11_INPUT_ELEMENT_DESC gLineVLayout[3] = { 0 };
global u32 gLineVLayoutCount = 3;

typedef enum cpu_access cpu_access;
enum cpu_access 
{
  Access_None  = 0,
  Access_Read  = D3D11_CPU_ACCESS_READ,
  Access_Write = D3D11_CPU_ACCESS_WRITE,
};

typedef enum buffer_usage buffer_usage;
enum buffer_usage 
{
  Usage_Default = D3D11_USAGE_DEFAULT,
  Usage_Dynamic = D3D11_USAGE_DYNAMIC,
};

fn void D3D11VertexBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data,
                          u32 Stride, u32 Count, buffer_usage Usage, cpu_access Access)
{
  // Normal vertex buffer fed in via the input assembler as point topology
  HRESULT Result;
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth      = Count*Stride;
  Desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
  Desc.Usage          = Usage;
  Desc.CPUAccessFlags = Access;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  Result = ID3D11Device_CreateBuffer(Device, &Desc, (Data==NULL)?NULL:&Initial, Buffer);
  Assert(SUCCEEDED(Result));
  return;
}
fn void D3D11IndexBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data,
                         u32 Stride, u32 Count, buffer_usage Usage, cpu_access Access)
{
  // Normal vertex buffer fed in via the input assembler as point topology
  HRESULT Result;
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth      = Count*Stride;
  Desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
  Desc.Usage          = Usage;
  Desc.CPUAccessFlags = Access;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  Result = ID3D11Device_CreateBuffer(Device, &Desc, (Data==NULL)?NULL:&Initial, Buffer);
  Assert(SUCCEEDED(Result));
  return;
}
fn void D3D11ConstantBuffer(ID3D11Device* Device, ID3D11Buffer **Buffer, void *Data,
                            u32 Size, buffer_usage Usage, cpu_access Access)
{
  HRESULT Result;
  D3D11_BUFFER_DESC Desc = {0};
  Desc.ByteWidth      = Size;
  Desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
  Desc.Usage          = Usage;
  Desc.CPUAccessFlags = Access;
  D3D11_SUBRESOURCE_DATA Initial;
  Initial.pSysMem = Data;
  Result = ID3D11Device_CreateBuffer(Device, &Desc, (Data==NULL)?NULL:&Initial, Buffer);
  Assert(SUCCEEDED(Result));
  return;
}
fn void D3D11InitTextureMapping(renderer                 *Renderer,
                                ID3D11Texture2D **Texture,
                                DXGI_FORMAT Format,
                                ID3D11ShaderResourceView **ResView,
                                ID3D11SamplerState       **SamplerState,
                                u32 SubresourceArraySize,
                                bitmapdata *BitmapData)
{
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
fn void OSGraphicsCleanup(renderer *Renderer)
{
  if(Renderer->RenderTargetView) ID3D11RenderTargetView_Release(Renderer->RenderTargetView);
  if(Renderer->SwapChain       ) IDXGISwapChain1_Release(Renderer->SwapChain);
  if(Renderer->Context         ) ID3D11DeviceContext_Release(Renderer->Context);
  if(Renderer->Device          ) ID3D11Device_Release(Renderer->Device);
  return;
}
fn void OSGraphicsInit(renderer *Renderer, v2s WindowDim, HWND Window)
{
  HRESULT Status;
  Renderer->Arena = ArenaInit(NULL, Kilobytes(2), Renderer->ArenaBuffer);
  Renderer->WindowDim.x = WindowDim.x;
  Renderer->WindowDim.y = WindowDim.y;
  D3D_FEATURE_LEVEL Levels[] = {D3D_FEATURE_LEVEL_11_0};
  UINT Flags = (D3D11_CREATE_DEVICE_BGRA_SUPPORT   |
                D3D11_CREATE_DEVICE_SINGLETHREADED |
                D3D11_CREATE_DEVICE_DEBUG);
  // NOTE(MIGUEL): For if I want to ceate the device and swapchain seperately.
  Status = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, Flags, Levels,
                             ArrayCount(Levels), D3D11_SDK_VERSION,
                             &Renderer->Device, 0, &Renderer->Context);
  Assert(SUCCEEDED(Status));
  // NOTE(MIGUEL): The swapchain BufferCount needs to be 2 to get
  //               2 backbuffers. Change it to 1 to see the effects.
  DXGI_SWAP_CHAIN_DESC1 SwapChainDescription = {0};
  SwapChainDescription.BufferCount = 2; 
  SwapChainDescription.Width  = Renderer->WindowDim.x;
  SwapChainDescription.Height = Renderer->WindowDim.y;
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
  
  ID3D11Texture2D* Backbuffer;
  IDXGISwapChain1_GetBuffer(Renderer->SwapChain, 0, &IID_ID3D11Texture2D, &Backbuffer);
  ID3D11Device_CreateRenderTargetView(Renderer->Device, (ID3D11Resource*)Backbuffer, NULL, &Renderer->RenderTargetView);
  ID3D11Texture2D_Release(Backbuffer);
  Assert(SUCCEEDED(Status));
  
  ID3D11DeviceContext_OMSetRenderTargets(Renderer->Context, 1, &Renderer->RenderTargetView, 0);
  D3D11_VIEWPORT Viewport;
  Viewport.TopLeftX = 0.0f;
  Viewport.TopLeftY = 0.0f;
  Viewport.Width  = (f32)Renderer->WindowDim.x;
  Viewport.Height = (f32)Renderer->WindowDim.y;
  Viewport.MinDepth = 0.0f;
  Viewport.MaxDepth = 1.0f;
  ID3D11DeviceContext_RSSetViewports(Renderer->Context, 1, &Viewport);
  return;
}

#endif //DX11_H
