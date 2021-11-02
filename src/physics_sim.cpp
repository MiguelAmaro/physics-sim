// ConsoleApplication1.cpp : Defines the entry point for the console application.

#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <windows.h>
#include <stdint.h>

#include <d3d11.h>
#include <d3dcompiler.h>
//#include <d3dx11.h>
//#include <d3d9.h>
//#include <DxErr.h>
//#include <D3dx9math.h>

#include "physics_sim_types.h"
#include "physics_sim_math.h"
#include "physics_sim_memory.h"
//#include "physics_sim_draw.h"
//#include "shared_file_out.h"
#define KILOBYTES(size) (         (size) * 1024LL)

#define MEGABYTES(size) (KILOBYTES(size) * 1024LL)
#define GIGABYTES(size) (MEGABYTES(size) * 1024LL)
#define TERABYTES(size) (GIGABYTES(size) * 1024LL)


//LPDIRECT3D9       g_Direct3D     = NULL;

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

struct app_state
{
    ID3D11Device           *Device;
    ID3D11DeviceContext    *Context;
    IDXGISwapChain         *SwapChain;
    D3D_FEATURE_LEVEL       FeatureLevel;
    ID3D11RenderTargetView *RenderTarget;
    
    ID3D11VertexShader *VertexShader;
    ID3D11PixelShader  *PixelShader;
    ID3D11InputLayout  *InputLayout;
    ID3D11Buffer       *VertexBuffer;
};

struct vertex
{
    v3f32 e;
    u32   color;
};


struct win32_WindowDim
{
    u32 Width;
    u32 Height;
};

win32_WindowDim g_WindowDim;

uint32_t g_Running = true;


#define DX_GET_ERROR_DESCRIPTION(name) const char* WINAPI name(__in HRESULT hr)
typedef DX_GET_ERROR_DESCRIPTION(dx_get_error_descriptiona);
DX_GET_ERROR_DESCRIPTION(dx_get_error_descriptiona_stub)
{ return "Error: DXGetErrorDescriptionA did not load!!"; }

dx_get_error_descriptiona *DXGetErrorDescriptionA_ = dx_get_error_descriptiona_stub;



static b32
D3D9GetDebugCapabilities(void)
{
    b32 Result = 1;
    
    // NOTE(MIGUEL): LoadLibrary only load Dynamically linked Libraries
    //               not Statically linked Libraries.
    
    //HMODULE Library = LoadLibraryA("F:\\Dev_Tools\\DirectXSDKLegacy\\Lib\\x64\\DxErr.lib");
    //DXGetErrorDescriptionA_ = (dx_get_error_descriptiona *)GetProcAddress(Library, "DXGetErrorDescriptionA"); 
    
    //Result = (DXGetErrorDescriptionA_ != dx_get_error_descriptiona_stub);
    
    return Result;
}

static void
D3D11Release(app_state *AppState)
{
    if(AppState->RenderTarget) AppState->RenderTarget->Release();
    if(AppState->SwapChain   ) AppState->SwapChain->Release();
    if(AppState->Context     ) AppState->Context->Release();
    if(AppState->Device      ) AppState->Device->Release();
    
    return;
}

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static b32
D3D11Init(HWND Window, app_state *AppState)
{
    HRESULT Status;
    b32     Result = true;
    
    UINT Flags = (D3D11_CREATE_DEVICE_BGRA_SUPPORT   |
                  D3D11_CREATE_DEVICE_SINGLETHREADED |
                  D3D11_CREATE_DEVICE_DEBUG);
    
    D3D_FEATURE_LEVEL Levels[] = {D3D_FEATURE_LEVEL_11_0};
    
#if 0
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
                                           &AppState->SwapChain,
                                           &AppState->Device,
                                           &AppState->FeatureLevel, &AppState->Context );
    
    ASSERT(SUCCEEDED(Status));
    
    
    ID3D11Texture2D        *BackBufferTexture;
    
    AppState->SwapChain->GetBuffer(0,
                                   __uuidof(ID3D11Texture2D),
                                   (LPVOID *)&BackBufferTexture);
    
    Status = AppState->Device->CreateRenderTargetView(BackBufferTexture, 0,
                                                      &AppState->RenderTarget);
    
    BackBufferTexture->Release();
    
    ASSERT(SUCCEEDED(Status));
    
    
    AppState->Context->OMSetRenderTargets(1, &AppState->RenderTarget, 0);
    
    D3D11_VIEWPORT ViewPort;
    ViewPort.Width  = (f32)g_WindowDim.Width;
    ViewPort.Height = (f32)g_WindowDim.Height;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;
    ViewPort.TopLeftX = 0.0f;
    ViewPort.TopLeftY = 0.0f;
    
    AppState->Context->RSSetViewports(1, &ViewPort);
    
    
#if 0
    //D3DDISPLAYMODE        CurrentDisplay = {0};
    //D3DPRESENT_PARAMETERS PresentParams  = {0};
    
    //D3D9GetDebugCapabilities();
    
    //PresentParams.Windowed      = true; 
    //PresentParams.hDeviceWindow = Window; 
    //PresentParams.SwapEffect    = D3DSWAPEFFECT_DISCARD;
    
    //*Direct3D = Direct3DCreate9(D3D_SDK_VERSION);
    
    const char *Info = NULL;
    /*
    if(*Direct3D)
    {
        if(FAILED(Status = g_Direct3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &CurrentDisplay)))
        {
            Result = false;
        }
        
        if(FAILED(Status = g_Direct3D->CreateDevice(D3DADAPTER_DEFAULT, 
                                                    D3DDEVTYPE_HAL,
                                                    Window,
                                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                                    &PresentParams,
                                                    RenderDevice)))
        {
            Result = false;
            
            Info = DXGetErrorDescriptionA_(Status);
            
            OutputDebugString(Info);
        }
    }
    */
#endif
    
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
        } break;
        
        default:
        {
            Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }
    
    return Result;
}

#define DEFAULT_WINDOW_COORDX  10
#define DEFAULT_WINDOW_COORDY  10
#define DEFAULT_WINDOW_WIDTH  800
#define DEFAULT_WINDOW_HEIGHT 1000

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

#define VERTEX_TYPE_SPECIFIER (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#include <timeapi.h>
void
Update(app_state *AppState)
{
#if 0
    D3DXMATRIXA16 WorldMatrix;
    
    UINT CurrentTime = timeGetTime() % 1000;
    FLOAT RotationAngle = CurrentTime * (2.0f * D3DX_PI) / 1000.0f;
    D3DXMatrixRotationZ(&WorldMatrix, RotationAngle);
    
    AppState->RenderDevice->SetTransform(D3DTS_WORLD, &WorldMatrix);
    D3DXVECTOR3 EyePoint(0.0f, 3.0f, -5.0f);
    D3DXVECTOR3 LookAtPoint(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 UpDirection(0.0f, 1.0f, 0.0f);
    
    D3DXMATRIXA16 ViewMatrix;
    D3DXMatrixLookAtLH(&ViewMatrix, &EyePoint, &LookAtPoint, &UpDirection);
    AppState->RenderDevice->SetTransform(D3DTS_VIEW, &ViewMatrix);
    
    
    D3DXMATRIXA16 ProjectionMatrix;
    D3DXMatrixPerspectiveFovLH(&ProjectionMatrix,
                               D3DX_PI / 4,
                               1.0f, 1.0f, 100.0f);
    
    AppState->RenderDevice->SetTransform(D3DTS_PROJECTION, &ProjectionMatrix);
#endif
    
    return;
}

void
Render(app_state *AppState)
{
    if(AppState->Context)
    {
        v4f32 ClearColor = v4f32Init(0.20f, 0.20f, 0.25f, 1.0f);
        
        AppState->Context->ClearRenderTargetView(AppState->RenderTarget, ClearColor.e);
        
        u32 Stride = sizeof(v3f32);
        u32 Offset = 0;
        
        AppState->Context->IASetInputLayout(AppState->InputLayout);
        AppState->Context->IASetVertexBuffers(0, 1, &AppState->VertexBuffer, &Stride, &Offset);
        AppState->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        AppState->Context->VSSetShader(AppState->VertexShader, 0, 0);
        AppState->Context->PSSetShader(AppState->PixelShader , 0, 0);
        
        AppState->Context->Draw(3, 0);
        
        AppState->SwapChain->Present(0 , 0);
    }
    
    return;
}



void PhysicsSimulation(app_state *AppState, app_input *Input)
{
    // NOTE(MIGUEL): Passive transformation is a transform that changes the coordinate
    //               system. (World moving around you when walking to simulate you looking around)
    //               Active transformation does not change the coordinate system instead
    //               it changes the vectors in the coordinate system. (This moving in the enviorment.)
    //               Both can be used
    
    
    {
        HRESULT Result;
        
#if 0
        u32 VerticesSize = sizeof(vertex) * 3;
        
        vertex Vertices[] =
        {
            {v3f32Init( 0.5f,  0.5f, 0.5f), 0xFFff0000 },
            {v3f32Init( 0.5f, -0.5f, 0.5f), 0xFF0000ff },
            {v3f32Init(-0.5f, -0.5f, 0.5f), 0xFFffffff },
            
        };
#else
        u32 VerticesSize = sizeof(v3f32) * 3;
        
        v3f32 Vertices[] =
        {
            {v3f32Init( 0.5f,  0.5f, 0.5f)},
            {v3f32Init( 0.5f, -0.5f, 0.5f)},
            {v3f32Init(-0.5f, -0.5f, 0.5f)},
        };
#endif
        
        D3D11_BUFFER_DESC VertexDescriptor = { 0 };
        
        VertexDescriptor.Usage     = D3D11_USAGE_DEFAULT;
        VertexDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        VertexDescriptor.ByteWidth = VerticesSize;
        
        D3D11_SUBRESOURCE_DATA ResourceData = { 0 };
        ResourceData.pSysMem = Vertices;
        
        //ID3D11Buffer* VertexBuffer;
        Result = AppState->Device->CreateBuffer(&VertexDescriptor,
                                                &ResourceData,
                                                &AppState->VertexBuffer );
        ASSERT(!FAILED(Result));
        
        /// CREATE VERTEX SHADER
        ID3DBlob *VertexShaderBuffer;
        
        DWORD ShaderFlags =  D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
        
        ID3DBlob *ErrorBuffer;
        
        // NOTE(MIGUEL): D3DX11CompileFromFile is depricated
        HANDLE ShaderCodeHandle;
        u8     ShaderCode[1024] = { 0 };
        
        ShaderCodeHandle = CreateFileA("..\\sampleshader.fx",
                                       GENERIC_READ, 0, 0,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL,
                                       0);
        
        ReadFile(ShaderCodeHandle, &ShaderCode, 1024, 0, 0);
        CloseHandle(ShaderCodeHandle);
        
        Result = D3DCompile(ShaderCode,
                            1024,
                            0,
                            0, 0,
                            "VS_Main",
                            "vs_4_0",
                            ShaderFlags,
                            0,
                            &VertexShaderBuffer,
                            &ErrorBuffer);
        
        if(FAILED(Result))
        {
            OutputDebugString((LPCSTR)ErrorBuffer->GetBufferPointer());
            
            if(ErrorBuffer != 0)
            {
                ErrorBuffer->Release();
                ASSERT(0);
            }
        }
        
        Result = AppState->Device->CreateVertexShader(VertexShaderBuffer->GetBufferPointer(),
                                                      VertexShaderBuffer->GetBufferSize(), 0,
                                                      &AppState->VertexShader);
        
        if(FAILED(Result))
        {
            if(VertexShaderBuffer) VertexShaderBuffer->Release();
            
            ASSERT(0);
        }
        
        /// SET INPUT LAYOUT
        D3D11_INPUT_ELEMENT_DESC VertexLayout[1] = { 0 };
        
        VertexLayout[0].SemanticName         = "POSITION";
        VertexLayout[0].SemanticIndex        = 0;
        VertexLayout[0].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
        VertexLayout[0].InputSlot            = 0;
        VertexLayout[0].AlignedByteOffset    = 0;
        VertexLayout[0].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        VertexLayout[0].InstanceDataStepRate = 0;
#if 0
        VertexLayout[1].SemanticName         = "COLOR";
        VertexLayout[1].SemanticIndex        = 0;
        VertexLayout[1].Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
        VertexLayout[1].InputSlot            = 0;
        VertexLayout[1].AlignedByteOffset    = 0;
        VertexLayout[1].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        VertexLayout[1].InstanceDataStepRate = 0;
#endif
        u32 TotalLayoutElements = ARRAYSIZE(VertexLayout);
        
        Result = AppState->Device->CreateInputLayout(VertexLayout,
                                                     TotalLayoutElements,
                                                     VertexShaderBuffer->GetBufferPointer(),
                                                     VertexShaderBuffer->GetBufferSize(),
                                                     &AppState->InputLayout);
        
        if(VertexShaderBuffer) VertexShaderBuffer->Release();
        
        /// CREATE PIXEL SHADER
        
        ID3DBlob* PixelShaderBuffer = 0;
        //ID3DBlob* ErrorBuffer = 0;
        
        // NOTE(MIGUEL): Already read shader file so should be in ShaderCode buffer 
        
        Result = D3DCompile(ShaderCode,
                            1024,
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
        
        Result = AppState->Device->CreatePixelShader(PixelShaderBuffer->GetBufferPointer(),
                                                     PixelShaderBuffer->GetBufferSize(), 0,
                                                     &AppState->PixelShader);
        
        
        if(PixelShaderBuffer) PixelShaderBuffer->Release();
        
        if(FAILED(Result))
        {
            ASSERT(0);
        }
    }
    
    b32 g_Pause = 0;
    
    while (g_Running)
    {
        ProcessPendingMessages(Input);
        
        if(!g_Pause)
        {
            Update(AppState);
            
            Render(AppState);
        }
    }
    
    return;
}

// TODO(MIGUEL): CH 3 [Beginning DirectX 11 Game Programming] 
// TODO(MIGUEL): CH 4 [Physics Modeling for Game Programmers] 
void WinMainCRTStartup()
{
    HWND Window = CreateOutputWindow();
    
    
    app_memory AppMemory = { 0 };
    
    LPVOID BaseAddress = 0;
    
    AppMemory.PermanentStorageSize = PERMANENT_STORAGE_SIZE;
    AppMemory.TransientStorageSize = TRANSIENT_STORAGE_SIZE;
    
    AppMemory.MainBlockSize = (AppMemory.PermanentStorageSize +
                               AppMemory.TransientStorageSize);
    
    AppMemory.MainBlock = VirtualAlloc(BaseAddress, 
                                       (size_t)AppMemory.MainBlockSize,
                                       MEM_COMMIT | MEM_RESERVE,
                                       PAGE_READWRITE);
    
    AppMemory.PermanentStorage = ((uint8_t *)AppMemory.MainBlock);
    
    AppMemory.TransientStorage = ((uint8_t *)AppMemory.PermanentStorage +
                                  AppMemory.PermanentStorageSize);
    
    
    app_state *AppState = (app_state *)AppMemory.PermanentStorage;
    
    app_input Input;
    
    /*// NOTE(MIGUEL): NOT IN USE
    
    memory_arena RenderArena = { 0 };
    
    MemoryArenaInit(&RenderArena,
                    AppMemory.TransientStorageSize,
                    AppMemory.TransientStorage);
    
    memory_arena InputArena = { 0 };
    
    MemoryArenaInit(&InputArena,
                    GIGABYTES(1),
                    AppMemory.TransientStorage);
    */
    
    // NOTE(MIGUEL): Device is created for processing rasterization
    if(D3D11Init(Window, AppState))
    {
        PhysicsSimulation(AppState, &Input);
    }
    
    
    D3D11Release(AppState);
    
    ExitProcess(0);
    
    return;
}

// CRT stuff

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
