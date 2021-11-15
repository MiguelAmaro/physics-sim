// ConsoleApplication1.cpp : Defines the entry point for the console application.

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdint.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <timeapi.h>

#include "physics_sim_config.h"
#include "physics_sim_types.h"
#include "physics_sim_memory.h"
#include "physics_sim_math.h"


//-/ TODO & NOTE
/// Implementation Objectives
// TODO(MIGUEL): Wall genaration & and enclose window with walls
//               instead checkin pos agains centered window half dim
// TODO(MIGUEL): Implende simple collisiont detection
// TODO(MIGUEL): Rotatate entity to match vel vector direction
// TODO(MIGUEL): More Entities
// TODO(MIGUEL): Make entities colide with each other
// TODO(MIGUEL): Hotswapables sim code
// TODO(MIGUEL): Hotswapable shader code

/// Learning Objectives
// TODO(MIGUEL): CH 3 [Beginning DirectX 11 Game Programming] 
// TODO(MIGUEL): CH 4 [Physics Modeling for Game Programmers] 

//-/ MACROS

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define KILOBYTES(size) (         (size) * 1024LL)

#define MEGABYTES(size) (KILOBYTES(size) * 1024LL)
#define GIGABYTES(size) (MEGABYTES(size) * 1024LL)
#define TERABYTES(size) (GIGABYTES(size) * 1024LL)

//-/ TYPES

struct button_state
{
    b32 EndedDown;
    u32 HalfTransitionCount;
};

struct app_input
{
    button_state ConnectArduino; 
    button_state SpawnGraph;
};

// NOTE(MIGUEL): For constant buffer by update frequency(low, high, static)
struct gpu_const_high
{
    m4f32 World;
    f32   Time;
    f32   _padding[3];
};

struct gpu_const_low
{
    m4f32  Proj;
};

struct gpu_const_static
{
    m4f32  View;
};

struct renderer
{
    ID3D11Device           *Device;
    ID3D11DeviceContext    *Context;
    IDXGISwapChain         *SwapChain;
    D3D_FEATURE_LEVEL       FeatureLevel;
    
    ID3D11RenderTargetView *TargetView;
    ID3D11VertexShader     *VertexShader;
    ID3D11PixelShader      *PixelShader;
    ID3D11InputLayout      *InputLayout;
    ID3D11Buffer           *VertexBuffer;
    
    ID3D11Buffer *CBHigh;
    ID3D11Buffer *CBLow;
    ID3D11Buffer *CBStatic;
    
    gpu_const_high   ConstBufferHigh;
    gpu_const_low    ConstBufferLow;
    gpu_const_static ConstBufferStatic;
};

enum entity_type
{
    Entity_Wall,
    Entity_Moves,
};

struct entity
{
    entity_type Type;
    
    v3f32 Pos;
    v3f32 Vel;
    v3f32 Acc;
    
    v3f32 Dim; /// Unit: Meters
    
    f32 EulerX;
    f32 EulerY;
    f32 EulerZ;
};

struct app_state
{
    b32 IsInitialized;
    f32 DeltaTimeMS;
    f32 Time;
    
    renderer Renderer;
    
    entity Entities[256];
    u32 EntityCount;
    u32 EntityMaxCount;
};

struct vertex
{
    v3f32 pos;
    v4f32 color;
};


struct win32_WindowDim
{
    u32 Width;
    u32 Height;
};


struct buffer
{
    u32 Width;
    u32 Height;
    u32 BytesPerPixel;
    void *Data;
};

//-/ GLOBALS

win32_WindowDim g_WindowDim;
b32             g_WindowResized;

uint32_t g_Running = true;
uint32_t g_Pause   = false;

//-/ FUNCTIONS

static void
D3D11InitTextureMappingCrap(renderer *Renderer, buffer *Buffer, u32 Width, u32 Height)
{
    D3D11_TEXTURE2D_DESC TextDesc = { 0 };
    TextDesc.Width  = Buffer->Width;
    TextDesc.Height = Buffer->Height;
    TextDesc.MipLevels = 1;
    TextDesc.ArraySize = 1;
    TextDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    TextDesc.SampleDesc.Count   = 1;
    TextDesc.SampleDesc.Quality = 0;
    TextDesc.Usage     = D3D11_USAGE_DEFAULT;
    TextDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    TextDesc.CPUAccessFlags = 0;
    TextDesc.MiscFlags      = 0;
    
    D3D11_SUBRESOURCE_DATA ResData = { 0 };
    ResData.pSysMem     = Buffer->Data;
    ResData.SysMemPitch = Buffer->Width * Buffer->BytesPerPixel;
    
    // TODO(MIGUEL): Figure out where to throw this
    ID3D11Texture2D *Texture;
    
    Renderer->Device->CreateTexture2D(&TextDesc, &ResData, &Texture);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResViewDesc = { 0 };
    ResViewDesc.Format          = TextDesc.Format;
    ResViewDesc.ViewDimension   = D3D11_SRV_DIMENSION_TEXTURE2D;
    ResViewDesc.Texture2D.MostDetailedMip = 0;
    ResViewDesc.Texture2D.MipLevels       = 1;
    
    ID3D11ShaderResourceView *ResView;
    
    Renderer->Device->CreateShaderResourceView(Texture, &ResViewDesc, &ResView);
    
    ID3D11SamplerState *SamplerState;
    
    D3D11_SAMPLER_DESC SamplerDesc = { 0 };
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    
    Renderer->Device->CreateSamplerState(&SamplerDesc, &SamplerState);
    
    // TODO(MIGUEL): Move to render()
    Renderer->Context->PSSetSamplers(0, 1, &SamplerState);
    
    return;
}

static void
D3D11Release(renderer *Renderer)
{
    if(Renderer->TargetView) Renderer->TargetView->Release();
    if(Renderer->SwapChain ) Renderer->SwapChain->Release();
    if(Renderer->Context   ) Renderer->Context->Release();
    if(Renderer->Device    ) Renderer->Device->Release();
    
    return;
}

static b32
D3D11Init(HWND Window, renderer *Renderer)
{
    HRESULT Status;
    b32     Result = true;
    
    UINT Flags = (D3D11_CREATE_DEVICE_BGRA_SUPPORT   |
                  D3D11_CREATE_DEVICE_SINGLETHREADED |
                  D3D11_CREATE_DEVICE_DEBUG);
    
    D3D_FEATURE_LEVEL Levels[] = {D3D_FEATURE_LEVEL_11_0};
    
#if 0
    // NOTE(MIGUEL): For if I want to ceate the device and swapchain seperately.
    Status = D3D11CreateDevice(0,
                               D3D_DRIVER_TYPE_HARDWARE,
                               0,
                               Flags,
                               Levels,
                               ARRAYSIZE(Levels),
                               D3D11_SDK_VERSION,
                               &AppState->Device, 0, &AppState->Context);
    
    ASSERT(SUCCEEDED(Status));
#endif
    
    DXGI_SWAP_CHAIN_DESC SwapChainDescription = {0};
    SwapChainDescription.BufferCount = 1;
    SwapChainDescription.BufferDesc.Width  = g_WindowDim.Width;
    SwapChainDescription.BufferDesc.Height = g_WindowDim.Height;
    SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferDesc.RefreshRate.Numerator   = 60;
    SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDescription.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.OutputWindow       = Window;
    SwapChainDescription.Windowed           = true;
    SwapChainDescription.SampleDesc.Count   = 1;
    SwapChainDescription.SampleDesc.Quality = 0;
    
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
    
    ASSERT(SUCCEEDED(Status));
    
    
    ID3D11Texture2D        *BackBufferTexture;
    
    Renderer->SwapChain->GetBuffer(0,
                                   __uuidof(ID3D11Texture2D),
                                   (LPVOID *)&BackBufferTexture);
    
    Status = Renderer->Device->CreateRenderTargetView(BackBufferTexture, 0,
                                                      &Renderer->TargetView);
    
    BackBufferTexture->Release();
    
    ASSERT(SUCCEEDED(Status));
    
    
    Renderer->Context->OMSetRenderTargets(1, &Renderer->TargetView, 0);
    
    D3D11_VIEWPORT ViewPort;
    ViewPort.Width  = (f32)g_WindowDim.Width;
    ViewPort.Height = (f32)g_WindowDim.Height;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;
    ViewPort.TopLeftX = 0.0f;
    ViewPort.TopLeftY = 0.0f;
    
    Renderer->Context->RSSetViewports(1, &ViewPort);
    
    return Result;
}

static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch (Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            EndPaint(Window, &Paint);
        } break;
        
        case WM_CHAR:
        case WM_SIZE:
        {
            //PostThreadMessage(RenderThreadID, Message, WParam, LParam);
            RECT WindowDim;
            GetClientRect(Window, &WindowDim);
            g_WindowDim.Width  = WindowDim.right  - WindowDim.left;
            g_WindowDim.Height = WindowDim.bottom - WindowDim.top;
            
            g_WindowResized = true;
        } break;
        
        default:
        {
            Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

static HWND CreateOutputWindow()
{
    WNDCLASSEXW WindowClass = {};
    
    WindowClass.cbSize        = sizeof(WindowClass);
    WindowClass.lpfnWndProc   = &WindowProc;
    WindowClass.style         = CS_HREDRAW | CS_VREDRAW;
    WindowClass.hInstance     = GetModuleHandleW(NULL);
    WindowClass.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    WindowClass.lpszClassName = L"physicsim";
    
    
    HWND Result = {0};
    if(RegisterClassExW(&WindowClass))
    {
        // NOTE(casey): Martins says WS_EX_NOREDIRECTIONBITMAP is necessary to make
        // DXGI_SWAP_EFFECT_FLIP_DISCARD "not glitch on window resizing", and since
        // I don't normally program DirectX and have no idea, we're just going to
        // leave it here :)
        DWORD ExStyle = 0; //WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
        
        RECT WindowDim = { 0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };
        
        // NOTE(MIGUEL): Is this correct? What about Client Rect?
        AdjustWindowRect(&WindowDim,
                         WS_OVERLAPPEDWINDOW,
                         false);
        
        g_WindowDim.Width  = WindowDim.right  - WindowDim.left;
        g_WindowDim.Height = WindowDim.bottom - WindowDim.top;
        
        Result = CreateWindowExW(ExStyle,
                                 WindowClass.lpszClassName,
                                 L"Physics Simulation",
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 DEFAULT_WINDOW_COORDX, DEFAULT_WINDOW_COORDY,
                                 g_WindowDim.Width, g_WindowDim.Height,
                                 0, 0, WindowClass.hInstance, 0);
    }
    
    return Result;
}

void
ProcessKeyboardMessage(button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
    
    return;
}

void
ProcessPendingMessages(app_input *Input)
{
    MSG Message = {};
    
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                g_Running = false;
            }  break;
            /*
            case WM_MOUSEWHEEL:
            {
                input->mouse_wheel_delta = ((s16)(Message.wParam >> 16) / 120.0f);
                input->mouse_wheel_integral += input->mouse_wheel_delta;
            } break;
            */
            case WM_SYSKEYUP:
            
            case WM_SYSKEYDOWN:
            
            case WM_KEYDOWN:
            
            case WM_KEYUP:
            {
                uint32_t VKCode          = (uint32_t)Message.wParam;
                uint32_t WasDown         = ((Message.lParam & (1 << 30)) != 0);
                uint32_t IsDown          = ((Message.lParam & (1 << 31)) == 0);
                
                if(WasDown != IsDown)
                {
                    switch(VKCode)
                    {
                        case 'C':
                        {
                            ProcessKeyboardMessage(&Input->ConnectArduino, IsDown);
                        } break;
                        
                        case 'E':
                        {
                            ProcessKeyboardMessage(&Input->SpawnGraph, IsDown);
                        } break;
                        
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
                        } break;
                        
                        case VK_ESCAPE:
                        {
                        } break;
                        
                        case VK_SPACE: 
                        {
                        } break;
                    }
                    
                    if(IsDown)
                    {
                        
                        u32 AltKeyWasDown = ( Message.lParam & (1 << 29));
                        if((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            g_Running = false;
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

void
Update(app_state *AppState)
{
    renderer *Renderer = &AppState->Renderer;
    
    static f32 RotZ = 0.0f;
    RotZ += 0.2f;
    
    entity   *Entity   =  AppState->Entities;
    for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
    {
        if(Entity->Type == Entity_Moves)
        {
            // NOTE(MIGUEL): Equations of motion
            
            f32   Speed = 0.25f;
            v3f32 Drag  = {0.0f, 0.0f, 0.0f};
            v3f32 Acc   = Entity->Acc * 1.0f;
            v3f32 Vel   = Speed * Entity->Vel + Drag;
            
            v3f32 PosDelta = (0.5f * Acc * Square(AppState->DeltaTimeMS) +
                              Vel * AppState->DeltaTimeMS);
            
            Entity->Vel = Acc * AppState->DeltaTimeMS + Entity->Vel;
            
            
            v3f32 NormalizedVel = v3f32Normalize(Vel);
            Entity->EulerZ = ArcTan2(NormalizedVel.x, NormalizedVel.y);
            
            v2f32 TestPos = (Entity->Pos + PosDelta).xy;
            entity *TestEntity = AppState->Entities;
            for(u32 TestEntityIndex = 0; TestEntityIndex < AppState->EntityCount;
                TestEntityIndex++, TestEntity++)
            {
                if(Entity != TestEntity && TestEntity->Type == Entity_Wall)
                {
                    
                    rect_v2f32 TestEntityBounds = rect_v2f32CenteredDim(TestEntity->Dim.xy);
                    
                    if(rect_v2f32IsInside(TestEntityBounds, TestPos))
                    {
                        Entity->Vel.x *= -1.0f;
                        Entity->Vel.y *= -1.0f;
                        
                        // NOTE(MIGUEL): Find the wall
                        if(TestPos.x)
                        {
                            
                        }
                        
                        PosDelta.x *= -1.0f;
                        PosDelta.y *= -1.0f;
                        PosDelta.z *= -1.0f;
                    }
                }
            }
            
            Entity->Pos += PosDelta;
        }
    }
    
    if(g_WindowResized || !AppState->IsInitialized)
    {
        // NOTE(MIGUEL): Low Update Frequency
        m4f32 Proj = m4f32Orthographic(0.0f, g_WindowDim.Width, 0.0f, g_WindowDim.Height, 0.0f, 100.0f);
        
        AppState->Renderer.ConstBufferLow.Proj    = Proj;
        
        Renderer->Context->UpdateSubresource(Renderer->CBLow, 0, 0,
                                             &Renderer->ConstBufferLow, 0, 0);
        g_WindowResized = 0;
    }
    
    if(!AppState->IsInitialized)
    {
        // NOTE(MIGUEL): No Update Frequency
        m4f32 View = m4f32Viewport(v2f32Init(g_WindowDim.Width, g_WindowDim.Height)); 
        AppState->Renderer.ConstBufferStatic.View = View;
        
        Renderer->Context->UpdateSubresource(Renderer->CBStatic, 0, 0,
                                             &Renderer->ConstBufferStatic, 0, 0);
        
        AppState->IsInitialized = 1;
    }
    
    
    return;
}

void
Render(app_state *AppState)
{
    renderer *Renderer = &AppState->Renderer;
    
    if(Renderer->Context)
    {
        //                             R      G      B      A
        v4f32 ClearColor = v4f32Init(0.10f, 0.10f, 0.10f, 1.0f);
        
        Renderer->Context->ClearRenderTargetView(Renderer->TargetView, ClearColor.c);
        
        HRESULT Result;
        
        // NOTE(MIGUEL): Sets the Model and How to Shade it
        u32 Stride[] = {sizeof(vertex)};
        u32 Offset[] = { 0 };
        
        Renderer->Context->IASetVertexBuffers(0, 1, &Renderer->VertexBuffer, Stride, Offset);
        Renderer->Context->IASetInputLayout(Renderer->InputLayout);
        Renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        entity *Entity = AppState->Entities;
        for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
        {
            //if(Entity->Type == Entity_Moves)
            {
                // NOTE(MIGUEL): High Update Frequency
                m4f32 Trans  = m4f32Translation(Entity->Pos);
                m4f32 Rotate = m4f32Rotation(0.0f, 0.0f, Entity->EulerZ);
                m4f32 Scale  = m4f32Scale   (Entity->Dim.x,
                                             Entity->Dim.y, 1.0f);
                m4f32 World  = Rotate * Scale * Trans;
                
                
                // NOTE(MIGUEL): This is only because this constant buffer isnt set to
                //               dynamic.(UpdateSubresource() call)
                // TODO(MIGUEL): Create a compiled path for a dynamic constant buffer
                
                AppState->Renderer.ConstBufferHigh.Time   = AppState->DeltaTimeMS;
                AppState->Renderer.ConstBufferHigh.World  = World;
                
                Renderer->Context->UpdateSubresource(Renderer->CBHigh, 0, 0,
                                                     &Renderer->ConstBufferHigh, 0, 0);
                
                Renderer->Context->VSSetShader(Renderer->VertexShader, 0, 0);
                Renderer->Context->VSSetConstantBuffers(0, 1, &Renderer->CBHigh);
                Renderer->Context->VSSetConstantBuffers(1, 1, &Renderer->CBStatic);
                Renderer->Context->VSSetConstantBuffers(2, 1, &Renderer->CBLow);
                
                Renderer->Context->PSSetShader(Renderer->PixelShader , 0, 0);
                
                Renderer->Context->Draw(3, 0);
            }
        }
        
        
        Renderer->SwapChain->Present(0 , 0);
    }
    
    return;
}

void LoadAssets(renderer *Renderer)
{
    HRESULT Result;
    
    // NOTE(MIGUEL): Creating a Model. Ill Reuse this for all Entities.
    {
        vertex Vertices[] =
        {
            {v3f32Init( 0.0f,  0.5f, 0.5f), v4f32Init( 1.0f, 0.0f, 0.0f, 1.0f)},
            {v3f32Init( 0.5f, -0.5f, 0.5f), v4f32Init( 0.0f, 1.0f, 0.0f, 1.0f)},
            {v3f32Init(-0.5f, -0.5f, 0.5f), v4f32Init( 0.0f, 0.0f, 1.0f, 1.0f)},
        };
        
        u32 VerticesSize = sizeof(vertex) * 3;
        
        D3D11_BUFFER_DESC VertexDescriptor = { 0 };
        
        VertexDescriptor.Usage     = D3D11_USAGE_DEFAULT;
        VertexDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        VertexDescriptor.ByteWidth = VerticesSize;
        
        D3D11_SUBRESOURCE_DATA ResourceData = { 0 };
        ResourceData.pSysMem = Vertices;
        
        Result = Renderer->Device->CreateBuffer(&VertexDescriptor,
                                                &ResourceData,
                                                &Renderer->VertexBuffer );
        ASSERT(!FAILED(Result));
        
        
        // NOTE(MIGUEL): What is the difference between dynamic and default usage?
        //               Why cant i use updatesubresource using dynamic and how do
        //               i pass const buff data to the pipeline. 
        // https://docs.microsoft.com/en-us/windows/win32/direct3d11/how-to--use-dynamic-resources
        D3D11_BUFFER_DESC GPUConstantsDesc = { 0 };
        GPUConstantsDesc.Usage          = D3D11_USAGE_DEFAULT; //D3D11_USAGE_DYNAMIC;
        GPUConstantsDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        GPUConstantsDesc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE;
        GPUConstantsDesc.ByteWidth      = sizeof(gpu_const_high);
        
        D3D11_SUBRESOURCE_DATA GPUConstantsResource = { 0 };
        GPUConstantsResource.pSysMem = &Renderer->ConstBufferHigh;
        
        Result = Renderer->Device->CreateBuffer(&GPUConstantsDesc,
                                                &GPUConstantsResource,
                                                &Renderer->CBHigh);
        ASSERT(!FAILED(Result));
        
        GPUConstantsDesc.ByteWidth      = sizeof(gpu_const_low);
        GPUConstantsResource.pSysMem = &Renderer->ConstBufferLow;
        
        Result = Renderer->Device->CreateBuffer(&GPUConstantsDesc,
                                                &GPUConstantsResource,
                                                &Renderer->CBLow);
        ASSERT(!FAILED(Result));
        
        GPUConstantsDesc.ByteWidth      = sizeof(gpu_const_static);
        GPUConstantsResource.pSysMem = &Renderer->ConstBufferStatic;
        
        Result = Renderer->Device->CreateBuffer(&GPUConstantsDesc,
                                                &GPUConstantsResource,
                                                &Renderer->CBStatic);
        ASSERT(!FAILED(Result));
        
    }
    
    // NOTE(MIGUEL): Ways to draw a Model
    {
        /// CREATE VERTEX SHADER
        ID3DBlob *VertexShaderBuffer;
        
        DWORD ShaderFlags =  D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
        
        ID3DBlob *ErrorBuffer;
        
        // NOTE(MIGUEL): D3DX11CompileFromFile is depricated
        HANDLE ShaderCodeHandle;
        u8     ShaderCode[4096] = { 0 };
        u32    ShaderCodeSize = 4096;
        
        ShaderCodeHandle = CreateFileA("..\\src\\default.hlsl",
                                       GENERIC_READ, 0, 0,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL,
                                       0);
        
        ReadFile(ShaderCodeHandle, &ShaderCode, ShaderCodeSize, 0, 0);
        CloseHandle(ShaderCodeHandle);
        
        Result = D3DCompile(ShaderCode, ShaderCodeSize,
                            0, 0, 0, "VS_Main", "vs_4_0", ShaderFlags, 0,
                            &VertexShaderBuffer, &ErrorBuffer);
        
        if(FAILED(Result))
        {
            OutputDebugString((LPCSTR)ErrorBuffer->GetBufferPointer());
            
            if(ErrorBuffer != 0)
            {
                ErrorBuffer->Release();
                ASSERT(0);
            }
        }
        
        Result = Renderer->Device->CreateVertexShader(VertexShaderBuffer->GetBufferPointer(),
                                                      VertexShaderBuffer->GetBufferSize(), 0,
                                                      &Renderer->VertexShader);
        
        if(FAILED(Result))
        {
            if(VertexShaderBuffer) VertexShaderBuffer->Release();
            
            ASSERT(0);
        }
        
        /// SET INPUT LAYOUT
        D3D11_INPUT_ELEMENT_DESC VertexLayout[2] = { 0 };
        
        VertexLayout[0].SemanticName         = "POSITION";
        VertexLayout[0].SemanticIndex        = 0;
        VertexLayout[0].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
        VertexLayout[0].InputSlot            = 0;
        VertexLayout[0].AlignedByteOffset    = 0;
        VertexLayout[0].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        VertexLayout[0].InstanceDataStepRate = 0;
        
        VertexLayout[1].SemanticName         = "COLOR";
        VertexLayout[1].SemanticIndex        = 0;
        VertexLayout[1].Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
        VertexLayout[1].InputSlot            = 0;
        VertexLayout[1].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
        VertexLayout[1].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        VertexLayout[1].InstanceDataStepRate = 0;
        
        u32 TotalLayoutElements = ARRAYSIZE(VertexLayout);
        
        Result = Renderer->Device->CreateInputLayout(VertexLayout,
                                                     TotalLayoutElements,
                                                     VertexShaderBuffer->GetBufferPointer(),
                                                     VertexShaderBuffer->GetBufferSize(),
                                                     &Renderer->InputLayout);
        
        if(VertexShaderBuffer) VertexShaderBuffer->Release();
        
        /// CREATE PIXEL SHADER
        
        ID3DBlob* PixelShaderBuffer = 0;
        
        Result = D3DCompile(ShaderCode,
                            4096,
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
                ErrorBuffer->Release();
                ASSERT(0);
            }
        }
        
        Result = Renderer->Device->CreatePixelShader(PixelShaderBuffer->GetBufferPointer(),
                                                     PixelShaderBuffer->GetBufferSize(), 0,
                                                     &Renderer->PixelShader);
        
        
        if(PixelShaderBuffer) PixelShaderBuffer->Release();
        
        if(FAILED(Result))
        {
            ASSERT(0);
        }
    }
    
    return;
}

void PhysicsSim(app_state *AppState, app_input *Input)
{
    
    LoadAssets(&AppState->Renderer);
    
    // NOTE(MIGUEL): Passive transformation is a transform that changes the coordinate
    //               system. (World moving around you when walking to simulate you looking around)
    //               Active transformation does not change the coordinate system instead
    //               it changes the vectors in the coordinate system. (This moving in the enviorment.)
    //               Both can be used
    
    {
        // NOTE(MIGUEL): Sim Initialization
        AppState->EntityCount    =   0;
        AppState->EntityMaxCount = 256;
        
        // NOTE(MIGUEL): Entity Generation
        
        
        // NOTE(MIGUEL): Wall Entities
        
        entity *EntityWallLeft   = &AppState->Entities[AppState->EntityCount++];
        entity *EntityWallRight  = &AppState->Entities[AppState->EntityCount++];
        entity *EntityWallTop    = &AppState->Entities[AppState->EntityCount++];
        entity *EntityWallBottom = &AppState->Entities[AppState->EntityCount++];
        
        entity *Entity = nullptr;
        
        f32 CommonWidth = 100.0f;
        
        Entity = EntityWallLeft;
        Entity->Dim.x = CommonWidth;
        Entity->Dim.y = g_WindowDim.Height + 40.0f ; 
        Entity->Dim.z = CommonWidth;
        
        Entity->Pos.x = -1.0f * (g_WindowDim.Width / 2.0f) - (Entity->Dim.x / 2.0f);
        Entity->Pos.y = 0.0f;
        Entity->Pos.z = 0.0f;
        Entity->Type = Entity_Wall;
        
        Entity = EntityWallRight;
        Entity->Dim.x = CommonWidth;
        Entity->Dim.y = g_WindowDim.Height + 40.0f ; 
        Entity->Dim.z = CommonWidth;
        
        Entity->Pos.x = 1.0f * (g_WindowDim.Width / 2.0f) + (Entity->Dim.x / 2.0f);
        Entity->Pos.y = 0.0f;
        Entity->Pos.z = 0.0f;
        Entity->Type = Entity_Wall;
        
        Entity = EntityWallTop;
        Entity->Dim.x = g_WindowDim.Width + 0.0f;
        Entity->Dim.y = CommonWidth; 
        Entity->Dim.z = CommonWidth;
        
        Entity->Pos.x = 0.0f;
        Entity->Pos.y = 1.0f * (g_WindowDim.Height / 2.0f) + (Entity->Dim.y / 2.0f);
        Entity->Pos.z = 0.0f;
        Entity->Type = Entity_Wall;
        
        Entity = EntityWallTop;
        Entity->Dim.x = g_WindowDim.Width + 0.0f;
        Entity->Dim.y = CommonWidth; 
        Entity->Dim.z = CommonWidth;
        
        Entity->Pos.x = 0.0f;
        Entity->Pos.y = -1.0f * (g_WindowDim.Height / 2.0f) - (Entity->Dim.y / 2.0f);
        Entity->Pos.z = 0.0f;
        Entity->Type = Entity_Wall;
        
        
        // NOTE(MIGUEL): Moving Entities
        u32 VectorTableSize = ARRAYSIZE(NormalizedVectorTable);
        for(u32 EntityIndex = 0; EntityIndex < 40; EntityIndex++)
        {
            
            if(AppState->EntityCount < AppState->EntityMaxCount)
            {
                Entity = &AppState->Entities[AppState->EntityCount++];
                
                //SYSTEMTIME SysTime;
                //GetSystemTime(&SysTime);
                
                LARGE_INTEGER Counter;
                QueryPerformanceCounter(&Counter);
                
                u32 Random = Counter.LowPart;
                
                u32 SeedX = (Random + 4 * (EntityIndex << 3)) << 8;
                u32 SeedY = (Random * (AppState->EntityCount << 23)) << 3;
                f32 X = NormalizedVectorTable[SeedX % VectorTableSize];
                f32 Y = NormalizedVectorTable[SeedY % VectorTableSize];
                
                Entity->Pos.x =  (200.0f + EntityIndex) * X;
                Entity->Pos.y =  (200.0f + EntityIndex) * Y;
                Entity->Pos.z =  0.0f;
                
                Entity->Dim.x = 20.0f;
                Entity->Dim.y = 20.0f; 
                Entity->Dim.z = 20.0f;
                
                Entity->Acc.x = 0.0f;
                Entity->Acc.y = 0.0f;
                Entity->Acc.z = 0.0f;
                
                Entity->Vel.x = X;
                Entity->Vel.y = Y;
                Entity->Vel.z = 0.0f;
                Entity->Type = Entity_Moves;
            }
        };
        
        
    }
    
    
    b32 g_Pause = 0;
    
    LARGE_INTEGER TickFrequency;
    LARGE_INTEGER WorkStartTick;
    LARGE_INTEGER WorkEndTick;
    LARGE_INTEGER WorkTickDelta;
    LARGE_INTEGER MicrosElapsedWorking;
    f32 TargetFPS = 60.0f;
    u64 TargetMicrosPerFrame = 16666;
    
    QueryPerformanceFrequency(&TickFrequency);
    
    while (g_Running)
    {
        // NOTE(MIGUEL): Start Timer
        QueryPerformanceCounter  (&WorkStartTick);
        
        ProcessPendingMessages(Input);
        
        if(!g_Pause)
        {
            Update(AppState);
            
            Render(AppState);
        }
        
        QueryPerformanceCounter  (&WorkEndTick);
        WorkTickDelta.QuadPart     = WorkEndTick.QuadPart - WorkStartTick.QuadPart;
        WorkTickDelta.QuadPart     = WorkTickDelta.QuadPart * 1000000;
        MicrosElapsedWorking.QuadPart = WorkTickDelta.QuadPart / TickFrequency.QuadPart;
        
        // NOTE(MIGUEL): Wait
        LARGE_INTEGER WaitTickDelta;
        LARGE_INTEGER WaitStartTick;
        LARGE_INTEGER WaitEndTick   = WorkEndTick;
        LARGE_INTEGER MicrosElapsedWaiting = { 0 };
        
        while((MicrosElapsedWorking.QuadPart +
               MicrosElapsedWaiting.QuadPart )
              < TargetMicrosPerFrame)
        {
            QueryPerformanceCounter  (&WaitStartTick);
            
            u32 TimeToSleep = (TargetMicrosPerFrame -
                               MicrosElapsedWorking.QuadPart +
                               MicrosElapsedWaiting.QuadPart) / 1000;
            
            Sleep(TimeToSleep);
            
            QueryPerformanceCounter  (&WaitEndTick);
            WaitTickDelta.QuadPart         = WaitEndTick.QuadPart - WaitStartTick.QuadPart;
            WaitTickDelta.QuadPart         = WaitTickDelta.QuadPart * 1000000;
            MicrosElapsedWaiting.QuadPart += (WaitTickDelta.QuadPart / TickFrequency.QuadPart);
        }
        
        AppState->DeltaTimeMS  = (f32)((MicrosElapsedWorking.QuadPart + MicrosElapsedWaiting.QuadPart) / 1000);
        AppState->Time        += AppState->DeltaTimeMS;
        
    }
    
    return;
}

void WinMainCRTStartup()
{
    HWND Window = CreateOutputWindow();
    
    app_memory AppMemory = { 0 };
    
    LPVOID BaseAddress = 0;
    {
        AppMemory.PermanentStorageSize = PERMANENT_STORAGE_SIZE;
        AppMemory.TransientStorageSize = TRANSIENT_STORAGE_SIZE;
        
        AppMemory.MainBlockSize        = (AppMemory.PermanentStorageSize +
                                          AppMemory.TransientStorageSize);
        
        AppMemory.MainBlock            = VirtualAlloc(BaseAddress, 
                                                      (size_t)AppMemory.MainBlockSize,
                                                      MEM_COMMIT | MEM_RESERVE,
                                                      PAGE_READWRITE);
        
        AppMemory.PermanentStorage     = ((uint8_t *)AppMemory.MainBlock);
        
        AppMemory.TransientStorage     = ((uint8_t *)AppMemory.PermanentStorage +
                                          AppMemory.PermanentStorageSize);
        
    }
    app_state *AppState = (app_state *)AppMemory.PermanentStorage;
    
    app_input Input;
    
#if 0
    // NOTE(MIGUEL): NOT IN USE
    memory_arena RenderArena = { 0 };
    
    MemoryArenaInit(&RenderArena,
                    AppMemory.TransientStorageSize,
                    AppMemory.TransientStorage);
    
    memory_arena InputArena = { 0 };
    
    MemoryArenaInit(&InputArena,
                    GIGABYTES(1),
                    AppMemory.TransientStorage);
    
#endif
    
    // NOTE(MIGUEL): Device is created for processing rasterization
    if(D3D11Init(Window, &AppState->Renderer))
    {
        PhysicsSim(AppState, &Input);
    }
    
    
    D3D11Release(&AppState->Renderer);
    
    ExitProcess(0);
    
    return;
}

//~ CRT stuff
extern "C" int _fltused = 0x9875;

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