// ConsoleApplication1.cpp : Defines the entry point for the console application.

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdint.h>


#include <d3dcompiler.h>
#include <directxmath.h>
#include <timeapi.h>

#include "physics_sim_config.h"
#include "physics_sim_blah.h"
#include "physics_sim_assets.h"
#include "physics_sim_types.h"
#include "physics_sim_memory.h"
#include "physics_sim_renderer.h"
#include "physics_sim_math.h"



/// SET INPUT LAYOUT
D3D11_INPUT_ELEMENT_DESC gVertexLayout[3] = { 0 };
u32 gVertexLayoutCount = 3;
D3D11_INPUT_ELEMENT_DESC gLineVLayout[3] = { 0 };
u32 gLineVLayoutCount = 3;



//-/ TODO & NOTE
/// Implementation Objectives
// TODO(MIGUEL): Wrap directx math funcitions in the mathe library. 
//               I dont have knowledge neccsary to do funtion approximations.
// TODO(MIGUEL): Font rendering(look at refterm)
// TODO(MIGUEL): Make entities colide with each other
// TODO(MIGUEL): Rotatate entity to match vel vector direction
// TODO(MIGUEL): Wavefront OBJ file parsing
// TODO(MIGUEL): Get app timing correct
// TODO(MIGUEL): Fix entity bugs on screen resize
// TODO(MIGUEL): Write a profiler

/// Learning Objectives
// TODO(MIGUEL): CH 3 [Beginning DirectX 11 Game Programming] 
// TODO(MIGUEL): CH 4 [Physics Modeling for Game Programmers] 


// IMPLEMENTED
// TODO(MIGUEL): Hotswapable shader code[DONE!]
// TODO(MIGUEL): Hotswapables sim code [DONE!]
// TODO(MIGUEL): More Entities[DONE!]
// TODO(MIGUEL): Implende simple collisiont detection [DONE!]
// TODO(MIGUEL): Wall genaration & and enclose window with walls
//               instead checkin pos agains centered window half dim [DONE!]

//-/ MACROS

#define WIN32_STATE_FILE_NAME_COUNT (MAX_PATH)


//-/ TYPES

struct bit_scan_result
{
    b32 Found;
    u32 Index;
};

struct win32_state
{
    char  ExeFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastExeFileNameSlash;
};


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

win32_state     g_winstate;
renderer        g_Renderer;
win32_WindowDim g_WindowDim;
b32             g_WindowResized;

uint32_t g_Running = true;
uint32_t g_Pause   = false;

//-/ FUNCTIONS
static u32
S8Length(const char *String)
{
    u32 Count = 0;
    
    while(*String++) { ++Count; }
    
    return Count;
}


void DrawString(const char *Message, f32 x, f32 y, renderer *Renderer)
{
    HRESULT Result;
    
    u32 CharLimit     = 24;
    //u32 SpriteSize    = sizeof(vertex) * ARRAY_COUNT(TextSpriteMeshVerts);
    u32 MessageLength = S8Length(Message);
    
    if(MessageLength > CharLimit)
    {
        MessageLength = CharLimit;
    }
    
    f32 CharWidth   = 32.0f / (f32)g_WindowDim.Width;
    f32 CharHeight  = 32.0f / (f32)g_WindowDim.Height;
    f32 TexelWidth  = 32.0f / 864.0f;
    //u32 VertsPerLetter = 6;
    
    // NOTE(MIGUEL): Updating A Dynamic Vertex Buffer
    D3D11_MAPPED_SUBRESOURCE MapResource;
    Result = Renderer->Context->Map(Renderer->TextSpriteVBuffer, 0,
                                    D3D11_MAP_WRITE_DISCARD, 0,
                                    &MapResource);
    ASSERT(!FAILED(Result));
    
    
    
    // Point to our vertex buffer’s internal data.
    vertex *SpritePtr     = (vertex *)MapResource.pData;
    
    
    u32 IndexA = (u32)'A';
    u32 IndexZ = (u32)'Z';
    
    Renderer->Context->IASetIndexBuffer(Renderer->TextSpriteIBuffer, DXGI_FORMAT_R16_UINT, 0 );
    for(u32 Index = 0; Index < MessageLength; ++Index)
    {
        f32 ThisStartX = x + (CharWidth * (f32)Index);
        f32 ThisEndX = ThisStartX + CharWidth;
        f32 ThisEndY = y + CharHeight;
        
        // NOTE(MIGUEL): Initializing the Mesh Values
        
        SpritePtr[0].Pos = v3f32Init(ThisEndX  , ThisEndY, 1.0f); // V0
        SpritePtr[1].Pos = v3f32Init(ThisEndX  , y       , 1.0f); // V1
        SpritePtr[2].Pos = v3f32Init(ThisStartX, y       , 1.0f); // V2
        SpritePtr[3].Pos = v3f32Init(ThisStartX, ThisEndY, 1.0f); // V3
        
        u32 TexLookUp = 0;
        u32 Letter = (u32)Message[Index];
        
        if(Letter < IndexA || Letter > IndexZ)
        {
            TexLookUp = (IndexZ - IndexA) + 1;
        }
        else
        {
            TexLookUp = (Letter - IndexA);
        }
        
        float TUStart = 01.0f + (TexelWidth * (f32)TexLookUp);
        float TUEnd   = TUStart + TexelWidth;
        
        SpritePtr[0].TexCoord = v2f32Init(TUEnd  , 0.0f); // V0
        SpritePtr[1].TexCoord = v2f32Init(TUEnd  , 1.0f); // V1
        SpritePtr[2].TexCoord = v2f32Init(TUStart, 1.0f); // V2
        SpritePtr[3].TexCoord = v2f32Init(TUStart, 0.0f); // V3
        
        SpritePtr += 4;
        
        Renderer->Context->DrawIndexed((6),0 , 4 * Index);
    }
    
    Renderer->Context->Unmap(Renderer->TextSpriteVBuffer, 0 );
    
    return;
}

bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {0};
    
    
#if COMPILER_MSVC
    Result.Found = _BitScanForward(&Result.Index, Value);
#else
    for(u32 Test = 0; Test < 32; ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = 1;
            
            break;
        }
    }
    
#endif
    
    return Result;
}


static bitmapdata
LoadBitmap(const char *FileName)
{
    bitmapdata Result = { 0 };
    
    LARGE_INTEGER FileSize;
    
    HANDLE FileHandle = CreateFileA(FileName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0, 0);
    
    GetFileSizeEx(FileHandle, &FileSize);
    
    u32 FileSize32 = SafeTruncateu64(FileSize.QuadPart);
    
    void *Block = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    
    ASSERT(Block);
    
    if(FileSize32 > 0)
    {
        DWORD BytesRead;
        
        if(ReadFile(FileHandle, Block, FileSize32, &BytesRead, 0) &&
           (FileSize32 == BytesRead))
        { }
        else
        {
            // NOTE(MIGUEL): Failed file read!!!
            VirtualFree(Block, 0, MEM_RELEASE);
            Block = 0;
        }
        
        bitmapheader *Header = (bitmapheader *)Block;
        u32 *Pixels = (u32 *)((u8 *)Block + Header->BitmapOffset);
        
        Result.Pixels = Pixels;
        Result.Width  = Header->Width;
        Result.Height = Header->Height;
        Result.BytesPerPixel = (Header->BitsPerPixel / 8);
        
        if(Header->Compression > 0)
        {
            u32 RedMask   = Header->RedMask;
            u32 GreenMask = Header->GreenMask;
            u32 BlueMask  = Header->BlueMask;
            u32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
            
            bit_scan_result RedShift   = FindLeastSignificantSetBit(RedMask  );
            bit_scan_result GreenShift = FindLeastSignificantSetBit(GreenMask);
            bit_scan_result BlueShift  = FindLeastSignificantSetBit(BlueMask );
            bit_scan_result AlphaShift = FindLeastSignificantSetBit(AlphaMask);
            
            
            ASSERT(  RedShift.Found);
            ASSERT(GreenShift.Found);
            ASSERT( BlueShift.Found);
            ASSERT(AlphaShift.Found);
            
            u32 *SrcDest = Pixels;
            
            for(    s32 y = 0; y < Header->Height; y++)
            {
                for(s32 x = 0; x < Header->Width; x++)
                {
                    u32 c = *SrcDest;
                    *SrcDest  = ((((c >> AlphaShift.Index) & 0xFF) << 24)|
                                 (((c >>   RedShift.Index) & 0xFF) << 16)|
                                 (((c >> GreenShift.Index) & 0xFF) <<  8)|
                                 (((c >>  BlueShift.Index) & 0xFF) <<  0));
                    SrcDest++;
                }
            }
        }
    }
    \
    return Result;
}

static void
D3D11InitTextureMapping(renderer                 *Renderer,
                        ID3D11ShaderResourceView **ResView,
                        ID3D11SamplerState       **SamplerState,
                        bitmapdata               *Buffer)
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
    ResData.pSysMem     = Buffer->Pixels;
    ResData.SysMemPitch = Buffer->Width * Buffer->BytesPerPixel;
    
    // TODO(MIGUEL): Figure out where to throw this
    ID3D11Texture2D *Texture;
    
    Renderer->Device->CreateTexture2D(&TextDesc, &ResData, &Texture);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResViewDesc = { 0 };
    ResViewDesc.Format          = TextDesc.Format;
    ResViewDesc.ViewDimension   = D3D11_SRV_DIMENSION_TEXTURE2D;
    ResViewDesc.Texture2D.MostDetailedMip = 0;
    ResViewDesc.Texture2D.MipLevels       = 1;
    
    Renderer->Device->CreateShaderResourceView(Texture, &ResViewDesc, ResView);
    
    D3D11_SAMPLER_DESC SamplerDesc = { 0 };
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    
    Renderer->Device->CreateSamplerState(&SamplerDesc, SamplerState);
    
    // TODO(MIGUEL): Move to render()
    Renderer->Context->PSSetSamplers(0, 1, SamplerState);
    
    return;
}

static void
D3D11Release(renderer *Renderer)
{
    if(Renderer->RenderTargetView) Renderer->RenderTargetView->Release();
    if(Renderer->SwapChain       ) Renderer->SwapChain->Release();
    if(Renderer->Context         ) Renderer->Context->Release();
    if(Renderer->Device          ) Renderer->Device->Release();
    
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
    
    // NOTE(MIGUEL): The swapchain BufferCount needs to be 2 to get
    //               2 backbuffers. Change it to 1 to see the effects.
    
    DXGI_SWAP_CHAIN_DESC SwapChainDescription = {0};
    SwapChainDescription.BufferCount = 2; 
    SwapChainDescription.BufferDesc.Width  = g_WindowDim.Width;
    SwapChainDescription.BufferDesc.Height = g_WindowDim.Height;
    SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferDesc.RefreshRate.Numerator   = 60;
    SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDescription.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.OutputWindow       = Window;
    SwapChainDescription.Windowed           = true;
    SwapChainDescription.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
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
                                                      &Renderer->RenderTargetView);
    
    BackBufferTexture->Release();
    
    ASSERT(SUCCEEDED(Status));
    
    
    Renderer->Context->OMSetRenderTargets(1, &Renderer->RenderTargetView, 0);
    
    D3D11_VIEWPORT ViewPort;
    ViewPort.TopLeftX = 0.0f;
    ViewPort.TopLeftY = 0.0f;
    ViewPort.Width  = (f32)g_WindowDim.Width;
    ViewPort.Height = (f32)g_WindowDim.Height;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;
    
    Renderer->Context->RSSetViewports(1, &ViewPort);
    
    return Result;
}

void PrintLastSystemError(void)
{
    LPTSTR ErrorMsg;
    uint32_t ErrorCode    = GetLastError();
    uint32_t ErrorMsgLen = 0;
    
    ErrorMsgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM     ,
                                 NULL                           ,
                                 ErrorCode                      ,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (char *)&ErrorMsg, 0, NULL);
    
    OutputDebugStringA(ErrorMsg);
    //MessageBox(0, ErrorMsg, "Warning", MB_OK);
    
    LocalFree(ErrorMsg);
    
    return;
}

void PrintSystemMsg(UINT WinMsg)
{
    LPTSTR MsgStr;
    uint32_t MsgLen = 0;
    
    MsgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM     ,
                            NULL                           ,
                            WinMsg   ,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (char *)&MsgStr, 0, NULL);
    
    OutputDebugStringA(MsgStr);
    //MessageBox(0, ErrorMsg, "Warning", MB_OK);
    
    LocalFree(MsgStr);
    
    return;
}

static LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    //OutputDebugString("processingMsg1\n");
    /*
switch (Message)
{
    
    
    case WM_CLOSE:
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    } break;
    
    //case WM_CHAR:
    case WM_SIZE:
    {
        OutputDebugString("Size Msg...");
        
    } break;
    
    default:
    {
        PrintSystemMsg(Message);
} break;
}
    */
    Result = DefWindowProcW(Window, Message, WParam, LParam);
    
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

// NOTE(MIGUEL): HOTSWAPPING SHADERCODE
#if 0

#endif

// NOTE(MIGUEL): HOTSWAPPING UPDATECODE
#if 1
static FILETIME
win32_GetLastWriteTime(char *FileName)
{
    FILETIME LastWriteTime = { 0 };
    
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
    
    if(GetFileAttributesEx((const char *)FileName,
                           GetFileExInfoStandard,
                           &FileInfo))
    {
        LastWriteTime = FileInfo.ftLastWriteTime;
    }
    
    return LastWriteTime;
}




struct win32_sim_code
{
    HMODULE     SIM_DLL;
    SIM_Update *Update;
    b32         IsValid;
    FILETIME    DLLLastWriteTime;
};


static void
win32_GetExeFileName(win32_state *State)
{
    /*
    u32 FileNameSize = GetModuleFileNameA(0,
                                          (LPSTR)State->ExeFileName,
                                          sizeof(State->ExeFileName));
    */
    State->OnePastLastExeFileNameSlash = State->ExeFileName;
    
    for(char *Scan = (char *)State->ExeFileName; *Scan; ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastExeFileNameSlash = Scan + 1;
        }
    }
    
    return;
}

static void
S8Concat(size_t SourceACount, char *SourceA,
         size_t SourceBCount, char *SourceB,
         size_t DestCount   , char *Dest    )
{
    // TODO(MIGUEL): Dest bounds checking!
    
    for(u32 Index = 0; Index < SourceACount; Index++)
    {
        *Dest++ = *SourceA++;
    }
    
    for(u32 Index = 0; Index < SourceBCount; Index++)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}


static void
win32_BuildExePathFileName(win32_state *State,
                           char *FileName,
                           size_t DestCount, char *Dest)
{
    S8Concat(State->OnePastLastExeFileNameSlash - State->ExeFileName,
             State->ExeFileName,
             S8Length(FileName), FileName,
             DestCount, Dest);
    
    return;
}


static win32_sim_code
win32_HotLoadSimCode(char *SourceDLLName, char *TempDLLName, char *LockedFileName)
{
    win32_sim_code Result = { 0 };
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx((const char *)LockedFileName,
                            GetFileExInfoStandard,
                            &Ignored))
    {
        Result.DLLLastWriteTime = win32_GetLastWriteTime(SourceDLLName);
        
        CopyFile((const char *)SourceDLLName,
                 (const char *)TempDLLName, FALSE);
        Result.SIM_DLL = LoadLibraryA((const char *)TempDLLName);
        
        if(Result.SIM_DLL)
        {
            Result.Update = (SIM_Update *)GetProcAddress(Result.SIM_DLL, "Update");
            
            Result.IsValid = (Result.Update != nullptr);
        }
    }
    if(!(Result.IsValid))
    {
        Result.Update = 0;
    }
    
    return Result;
}


static void
win32_HotUnloadSimCode(win32_sim_code *Sim)
{
    if(Sim->SIM_DLL)
    {
        FreeLibrary(Sim->SIM_DLL);
    }
    
    Sim->IsValid = false;
    Sim->Update  = 0;
    
    return;
}
#endif

void
ProcessPendingMessages(app_input *Input)
{
    MSG Message = {};
    
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        //PrintSystemMsg(Message.message);
        
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


void CircleGeometry(v3f32 *Buffer, u32 BufferSize, u16 *IBuffer, u32 IBufferSize, u32 Resolution,
                    u32 *VertCountResult, u32 *IndexCountResult)
{
    v3f32 *BufferEnd = (v3f32 *)((u8 *)Buffer + BufferSize);
    v3f32 *Vert = Buffer;
    u32 VertCount = 0;
    
    //ORIGIN
    *Vert = {0.0f, 0.0f, 0.0f};
    Vert++; VertCount++;
    
    for(u32 Wedge = 0; Wedge<Resolution; Wedge++)
    {
        f32 Theta = (2.0f * PI32 * (f32)Wedge) / (f32)Resolution;
        if((Vert + 1) < BufferEnd)
        {
            f32 CT = Cosine(Theta);
            f32 ST = Sine(Theta);
            *Vert = {0.5f * CT, 0.5f * ST, 0.0f};
            Vert++; VertCount++;
        }
        else
        {
            ASSERT("Out of Memory!!");
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
            ASSERT("Out of Memory!!");
        }
    }
    
    if(VertCountResult ) *VertCountResult  = VertCount;
    if(IndexCountResult) *IndexCountResult = IndexCount;
    
    return;
}


void DrawLine(renderer *Renderer,
              app_state *AppState,
              f32 LineWidth,
              v3f32 *PointList,
              u32 PointCount,
              v4f32 Color)
{
    //-ROUNDJOINS
    u32 LineCount = PointCount - 1;
    u32 JoinCount = PointCount - 2;
    
    
    
    v3f32 JoinVerts  [16 + 1] = {0};
    u16    JoinIndeces[256] = {0};
    u32 JoinIndexCount = 0;
    CircleGeometry(JoinVerts, sizeof(JoinVerts),
                   JoinIndeces, sizeof(JoinIndeces),
                   15, 0, &JoinIndexCount);
    //-
    
    //-LINE VERT DATA
    v3f32 LineVData[512] = { 0 };
    MemoryCopy(LineVData, LineMeshVerts, sizeof(LineMeshVerts));
    MemoryCopy(LineVData+ARRAY_COUNT(LineMeshVerts), JoinVerts, sizeof(JoinVerts));
    
    
    D3D11_MAPPED_SUBRESOURCE InstanceMap = { 0 };
    Renderer->Context->Map(Renderer->LineVInstBuffer,
                           0, D3D11_MAP_WRITE_DISCARD, 0,
                           &InstanceMap);
    MemoryCopy(InstanceMap.pData, PointList, sizeof(v3f32)*PointCount);
    Renderer->Context->Unmap(Renderer->LineVInstBuffer, 0);
    
    D3D11_MAPPED_SUBRESOURCE Mapped = { 0 };
    Renderer->Context->Map(Renderer->LineVBuffer,
                           0, D3D11_MAP_WRITE_DISCARD, 0,
                           &Mapped);
    MemoryCopy(Mapped.pData, LineVData, sizeof(LineVData));
    Renderer->Context->Unmap(Renderer->LineVBuffer, 0);
    
    ID3D11Buffer *VBuffers[2] = { Renderer->LineVBuffer, Renderer->LineVInstBuffer };
    UINT          VStrides[2] = { sizeof(v3f32), sizeof(v3f32)};
    UINT          VOffsets[2] = { 0, 0};
    Renderer->Context->IASetVertexBuffers(0, 2, VBuffers, VStrides, VOffsets);
    Renderer->Context->IASetInputLayout(Renderer->LineInputLayout);
    Renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    
    // NOTE(MIGUEL): This is only because this constant buffer isnt set to
    //               dynamic.(UpdateSubresource() call)
    // TODO(MIGUEL): Create a compiled path for a dynamic constant buffer
    
    
    Renderer->ConstBufferHigh.Time = AppState->Time;
    Renderer->ConstBufferHigh.Width  = LineWidth;
    Renderer->ConstBufferHigh.Color  = Color;
    Renderer->ConstBufferHigh.JoinType  = 0;
    
    D3D11_MAPPED_SUBRESOURCE MappedHigh = { 0 };
    Renderer->Context->Map(Renderer->CBHigh, 0,
                           D3D11_MAP_WRITE_DISCARD,
                           0, &MappedHigh);
    MemoryCopy(MappedHigh.pData, &Renderer->ConstBufferHigh, sizeof(gpu_const_high));
    Renderer->Context->Unmap(Renderer->CBHigh, 0);
    
    Renderer->Context->VSSetShader(Renderer->LineVShader, 0, 0);
    // NOTE(MIGUEL): The first arg is register number of the buffer this means
    //               that if in the shader the CB has : register(b0) then it should
    //               be 0.
    Renderer->Context->VSSetConstantBuffers(0, 1, &Renderer->CBLow);
    Renderer->Context->VSSetConstantBuffers(1, 1, &Renderer->CBHigh);
    
    Renderer->Context->PSSetShader(Renderer->LinePShader, 0, 0);
    Renderer->Context->PSSetConstantBuffers(0, 1, &Renderer->CBLow);
    Renderer->Context->PSSetConstantBuffers(1, 1, &Renderer->CBHigh);
    
    Renderer->Context->DrawInstanced(ARRAY_COUNT(LineMeshVerts), LineCount, 0, 0);
    
    //-
    D3D11_MAPPED_SUBRESOURCE JIMapped = { 0 };
    Renderer->Context->Map(Renderer->LineIBuffer,
                           0, D3D11_MAP_WRITE_DISCARD,
                           0, &JIMapped);
    MemoryCopy(JIMapped.pData, JoinIndeces, sizeof(u16)*JoinIndexCount);
    Renderer->Context->Unmap(Renderer->LineIBuffer, 0);
    
    Renderer->ConstBufferHigh.Time = AppState->Time;
    Renderer->ConstBufferHigh.Width  = LineWidth;
    Renderer->ConstBufferHigh.Color  = Color;
    Renderer->ConstBufferHigh.JoinType  = 1;
    
    Renderer->Context->Map(Renderer->CBHigh, 0,
                           D3D11_MAP_WRITE_DISCARD,
                           0, &MappedHigh);
    MemoryCopy(MappedHigh.pData, &Renderer->ConstBufferHigh, sizeof(gpu_const_high));
    Renderer->Context->Unmap(Renderer->CBHigh, 0);
    
    ID3D11Buffer *JVBuffers[2] = { Renderer->LineVBuffer, Renderer->LineVInstBuffer };
    UINT          JVStrides[2] = { sizeof(v3f32), sizeof(v3f32)};
    UINT          JVOffsets[2] = { sizeof(LineMeshVerts), 0};
    
    Renderer->Context->IASetVertexBuffers(0, 2, JVBuffers, JVStrides, JVOffsets);
    Renderer->Context->IASetInputLayout(Renderer->LineInputLayout);
    Renderer->Context->IASetIndexBuffer(Renderer->LineIBuffer, DXGI_FORMAT_R16_UINT, 0 );
    Renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    
    Renderer->Context->VSSetShader(Renderer->LineVShader, 0, 0);
    Renderer->Context->VSSetConstantBuffers(0, 1, &Renderer->CBLow);
    Renderer->Context->VSSetConstantBuffers(1, 1, &Renderer->CBHigh);
    
    Renderer->Context->PSSetShader(Renderer->LinePShader, 0, 0);
    Renderer->Context->PSSetConstantBuffers(0, 1, &Renderer->CBLow);
    Renderer->Context->PSSetConstantBuffers(1, 1, &Renderer->CBHigh);
    
    Renderer->Context->DrawIndexedInstanced(JoinIndexCount, JoinCount, 0, 0, 0);
    
    return;
}


void
Render(renderer *Renderer, app_memory *AppMemory)
{
    app_state *AppState = (app_state *)AppMemory->PermanentStorage;
    
    
    if(Renderer->Context)
    {
        if(g_WindowResized || !AppState->IsInitialized)
        {
            
            m4f32 Proj = m4f32Ortho(0.0f, (f32)g_WindowDim.Width, 0.0f,
                                    (f32)g_WindowDim.Height, 0.0f, 100.0f);
            
            m4f32 View = m4f32Viewport(v2f32Init((f32)g_WindowDim.Width,
                                                 (f32)g_WindowDim.Height)); 
            Renderer->ConstBufferLow.Proj    = Proj;
            Renderer->ConstBufferLow.View    = View;
            Renderer->ConstBufferLow.Res    = {(f32)g_WindowDim.Width, (f32)g_WindowDim.Height};
            
            
            Renderer->Context->OMSetRenderTargets(0, 0, 0);
            Renderer->RenderTargetView->Release();
            Renderer->SwapChain->ResizeBuffers(0,
                                               (u32)g_WindowDim.Width,
                                               (u32)g_WindowDim.Height,
                                               DXGI_FORMAT_UNKNOWN, 0);
            
            ID3D11Texture2D        *BackBufferTexture;
            Renderer->SwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),
                                           (LPVOID *)&BackBufferTexture);
            
            Renderer->Device->CreateRenderTargetView(BackBufferTexture, 0,
                                                     &Renderer->RenderTargetView);
            BackBufferTexture->Release();
            
            Renderer->Context->OMSetRenderTargets(1, &Renderer->RenderTargetView, 0);
            
            D3D11_MAPPED_SUBRESOURCE MappedConst = { 0 };
            Renderer->Context->Map(Renderer->CBLow, 0,
                                   D3D11_MAP_WRITE_DISCARD,
                                   0, &MappedConst);
            
            MemoryCopy(MappedConst.pData, &Renderer->ConstBufferLow, sizeof(gpu_const_low));
            Renderer->Context->Unmap(Renderer->CBLow, 0);
            
            g_WindowResized = 0;
        }
        
        
        Renderer->Context->OMSetRenderTargets(1, &Renderer->RenderTargetView, 0);
        
        D3D11_VIEWPORT Viewport;
        Viewport.TopLeftY = 0;
        Viewport.TopLeftX = 0;
        Viewport.Width  = (f32)g_WindowDim.Width ;
        Viewport.MinDepth  = 0;
        Viewport.MaxDepth  = 0;
        Viewport.Width  = (f32)g_WindowDim.Width ;
        Viewport.Height = (f32)g_WindowDim.Height;
        
        Renderer->Context->RSSetViewports(1, &Viewport );
        
        
        v4f32 ClearColor = v4f32Init(0.4f, 0.4f, 0.4f, 1.0f);
        
        Renderer->Context->ClearRenderTargetView(Renderer->RenderTargetView, ClearColor.c);
        
#if 1
        render_buffer *RendererBuffer = &Renderer->RenderBuffer;
        render_entry *RenderEntry    =  Renderer->RenderBuffer.Entries;
        
        for(u32 Entry = 0; Entry < RendererBuffer->EntryCount; Entry++, RenderEntry++)
        {
            switch(RenderEntry->Type)
            {
                case RenderType_quad:
                {
                    // NOTE(MIGUEL): Sets the Model and How to Shade it
                    u32 Stride[] = {sizeof(vertex)};
                    u32 Offset[] = { 0 };
                    
                    Renderer->Context->IASetVertexBuffers(0, 1, &Renderer->QuadVBuffer, Stride, Offset);
                    Renderer->Context->IASetInputLayout(Renderer->InputLayout);
                    Renderer->Context->IASetIndexBuffer(Renderer->QuadIBuffer, DXGI_FORMAT_R16_UINT, 0 );
                    Renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    
                    v3f32 CosSin = *((v3f32 *)RenderEntry->Data);
                    
                    // NOTE(MIGUEL): High Update Frequency
                    m4f32 Trans  = m4f32Translate(RenderEntry->Pos);
                    m4f32 Rotate = m4f32Rotate2D(CosSin.x, CosSin.y);
                    m4f32 Scale  = m4f32Scale   (RenderEntry->Dim.x,
                                                 RenderEntry->Dim.y, 1.0f);
                    m4f32 World  = Trans  * Rotate * Scale;
                    
                    
                    // NOTE(MIGUEL): This is only because this constant buffer isnt set to
                    //               dynamic.(UpdateSubresource() call)
                    // TODO(MIGUEL): Create a compiled path for a dynamic constant buffer
                    
                    
                    Renderer->ConstBufferHigh.Time   = AppState->Time;
                    Renderer->ConstBufferHigh.World  = World;
                    
                    D3D11_MAPPED_SUBRESOURCE QuadVBufferMap = { 0 };
                    Renderer->Context->Map(Renderer->QuadVBuffer, 0,
                                           D3D11_MAP_WRITE_DISCARD,
                                           0, &QuadVBufferMap);
                    MemoryCopy(QuadVBufferMap.pData, QuadMeshVerts, sizeof(QuadMeshVerts));
                    Renderer->Context->Unmap(Renderer->QuadVBuffer, 0);
                    
                    D3D11_MAPPED_SUBRESOURCE QuadIBufferMap = { 0 };
                    Renderer->Context->Map(Renderer->QuadIBuffer, 0,
                                           D3D11_MAP_WRITE_DISCARD,
                                           0, &QuadIBufferMap);
                    MemoryCopy(QuadIBufferMap.pData, QuadMeshIndices, sizeof(QuadMeshIndices));
                    Renderer->Context->Unmap(Renderer->QuadIBuffer, 0);
                    
                    D3D11_MAPPED_SUBRESOURCE MappedConst = { 0 };
                    Renderer->Context->Map(Renderer->CBHigh, 0,
                                           D3D11_MAP_WRITE_DISCARD,
                                           0, &MappedConst);
                    MemoryCopy(MappedConst.pData, &Renderer->ConstBufferHigh, sizeof(gpu_const_high));
                    Renderer->Context->Unmap(Renderer->CBHigh, 0);
                    
                    
                    Renderer->Context->VSSetShader(Renderer->VertexShader, 0, 0);
                    Renderer->Context->VSSetConstantBuffers(0, 1, &Renderer->CBHigh);
                    Renderer->Context->VSSetConstantBuffers(1, 1, &Renderer->CBLow);
                    
                    Renderer->Context->PSSetShader(Renderer->PixelShader , 0, 0);
                    Renderer->Context->PSSetShaderResources(0, 1, &Renderer->SmileyTexView);
                    Renderer->Context->PSSetSamplers       (0, 1, &Renderer->SmileySamplerState);
                    
                    Renderer->Context->DrawIndexed(ARRAY_COUNT(QuadMeshIndices), 0, 0);
                }break;
                case RenderType_tri:
                {} break;
                case RenderType_line:
                {
                    render_line *Line = (render_line *)RenderEntry->Data;
                    
                    DrawLine(Renderer, AppState, Line->Width * 1.75f, Line->PointData, Line->PointCount,
                             v4f32Init(0.0f, 0.0f, 0.0f, 1.0f));
                    DrawLine(Renderer, AppState, Line->Width, Line->PointData, Line->PointCount, Line->Color);
                    
                } break;
            }
        }
        
        /// TEXT
#if 0
        u32 TextSpriteStride[] = {sizeof(vertex)};
        u32 TextSpriteOffset[] = { 0 };
        
        Renderer->Context->IASetVertexBuffers(0, 1,
                                              &Renderer->TextSpriteVBuffer,
                                              TextSpriteStride,
                                              TextSpriteOffset);
        Renderer->Context->IASetInputLayout(Renderer->InputLayout);
        Renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        // NOTE(MIGUEL): High Update Frequency
        m4f32 Trans  = m4f32Translation(v3f32Init(100.0f, 100.0f, 1.0f));
        m4f32 Scale  = m4f32Scale   (800.0f, 800.0f, 1.0f);
        m4f32 Rotate = m4f32Rotation(0.0f, 0.0f, 0.0f);
        m4f32 World  = Rotate * Scale * Trans;
        
        Renderer->ConstBufferHigh.Time   = AppState->DeltaTimeMS;
        Renderer->ConstBufferHigh.World  = World;
        
        D3D11_MAPPED_SUBRESOURCE MappedConst = { 0 };
        Renderer->Context->Map(Renderer->CBHigh, 0,
                               D3D11_MAP_WRITE_DISCARD,
                               0, &MappedConst);
        
        MemoryCopy(MappedConst.pData, &Renderer->ConstBufferHigh, sizeof(gpu_const_high));
        Renderer->Context->Unmap(Renderer->CBHigh, 0);
        
        
        Renderer->Context->VSSetShader(Renderer->VertexShader, 0, 0);
        Renderer->Context->VSSetConstantBuffers(0, 1, &Renderer->CBHigh);
        Renderer->Context->VSSetConstantBuffers(1, 1, &Renderer->CBLow);
        
        
        Renderer->Context->VSSetShader(Renderer->VertexShader, 0, 0);
        Renderer->Context->PSSetShader(Renderer->PixelShader , 0, 0);
        Renderer->Context->PSSetShaderResources(0, 1, &Renderer->TextTexView);
        Renderer->Context->PSSetSamplers       (0, 1, &Renderer->TextSamplerState);
        
        // NOTE(MIGUEL): Is this function in clip space??
        DrawString("HELLO WORLD", -0.2f, 0.0f, Renderer);
#endif
#endif
        
#if 1
        //-LINE DATA
        f32 LineWidth = 5.0f;
        u32 PointCount = 0;
        v3f32 PointData[256 * 2] = { 0 };
        v4f32  Color = {0.0f, 0.0f, 0.0f, 1.0f};
        v4f32  Color1 = {1.0f, 1.0f, 1.0f, 1.0f};
        
        PointData[0] = v3f32Init(100.0, 100.0, 0.0);
        PointCount++;
        
        PointData[1] = v3f32Init(200.0f+ 50.0f*Cosine(AppState->Time*0.001f),
                                 200.0f+ 50.0f*Sine(AppState->Time*0.001f), 0.0f);
        PointCount++;
        
        PointData[2] = v3f32Init(300.0f+ 80.0f*Cosine(AppState->Time*0.004f),
                                 400.0f+ 50.0f*-Sine(AppState->Time*0.004f), 0.0f);
        PointCount++;
        PointData[3] = v3f32Init(400.0, 300.0, 0.0);
        PointCount++;
        PointData[4] = v3f32Init(100.0, 900.0, 0.0);
        PointCount++;
        DrawLine(Renderer, AppState, LineWidth, PointData, PointCount, Color);
        DrawLine(Renderer, AppState, LineWidth*0.2f, PointData, PointCount, Color1);
#endif
        //~/FLIP
        Renderer->SwapChain->Present(0 , 0);
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
               memory_arena *AssetLoadingArena)
{
    HRESULT Result;
    
    /// CREATE VERTEX SHADER
    ID3DBlob *VertexShaderBuffer;
    
    DWORD ShaderFlags =  D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
    
    ID3DBlob *ErrorBuffer;
    
    // NOTE(MIGUEL): D3DX11CompileFromFile is depricated
    
    u8 *ShaderCode = MEMORY_ARENA_PUSH_ARRAY(AssetLoadingArena,
                                             ShaderFileSize, u8);
    
    ReadFile(ShaderCodeHandle, ShaderCode, ShaderFileSize, 0, 0);
    CloseHandle(ShaderCodeHandle);
    
    Result = D3DCompile(ShaderCode, ShaderFileSize,
                        0, 0, 0, "VS_Main", "vs_4_0", ShaderFlags, 0,
                        &VertexShaderBuffer, &ErrorBuffer);
    
    if(FAILED(Result))
    {
        OutputDebugString((LPCSTR)ErrorBuffer->GetBufferPointer());
        
        if(ErrorBuffer != 0)
        {
            ErrorBuffer->Release();
            
            return false;
        }
    }
    
    Result = Renderer->Device->CreateVertexShader(VertexShaderBuffer->GetBufferPointer(),
                                                  VertexShaderBuffer->GetBufferSize(), 0,
                                                  NewVertexShader);
    
    if(FAILED(Result))
    {
        if(VertexShaderBuffer) VertexShaderBuffer->Release();
        
        return false;
    }
    
    
    Result = Renderer->Device->CreateInputLayout(ElemDesc,
                                                 ElemCount,
                                                 VertexShaderBuffer->GetBufferPointer(),
                                                 VertexShaderBuffer->GetBufferSize(),
                                                 InputLayout);
    
    if(VertexShaderBuffer) VertexShaderBuffer->Release();
    
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
            ErrorBuffer->Release();
            
            return false;
        }
    }
    
    Result = Renderer->Device->CreatePixelShader(PixelShaderBuffer->GetBufferPointer(),
                                                 PixelShaderBuffer->GetBufferSize(), 0,
                                                 NewPixelShader);
    
    
    if(PixelShaderBuffer) PixelShaderBuffer->Release();
    
    if(FAILED(Result))
    {
        
        return false;
    }
    
    return true; 
}

void D3D11LoadResources(renderer *Renderer, memory_arena *AssetLoadingArena)
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
        LineVDesc.ByteWidth = sizeof(v3f32) * 1049;
        
        Result = Renderer->Device->CreateBuffer(&LineVDesc,
                                                NULL,
                                                &Renderer->LineVBuffer);
        
        ASSERT(!FAILED(Result));
        
        D3D11_BUFFER_DESC LineVInstDesc = { 0 };
        LineVInstDesc.Usage = D3D11_USAGE_DYNAMIC;
        LineVInstDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        LineVInstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        LineVInstDesc.ByteWidth = 256 * (2 * sizeof(v3f32)); //for point a & b
        
        Result = Renderer->Device->CreateBuffer(&LineVInstDesc,
                                                NULL,
                                                &Renderer->LineVInstBuffer);
        
        ASSERT(!FAILED(Result));
        
        D3D11_BUFFER_DESC LineIDesc = { 0 };
        LineIDesc.Usage = D3D11_USAGE_DYNAMIC;
        LineIDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        LineIDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        LineIDesc.ByteWidth = 256 * sizeof(u16); //for point a & b
        
        Result = Renderer->Device->CreateBuffer(&LineIDesc,
                                                NULL,
                                                &Renderer->LineIBuffer);
        
        ASSERT(!FAILED(Result));
        
        /// TRIANGLE
        D3D11_BUFFER_DESC TriangleVertDesc = { 0 };
        TriangleVertDesc.Usage     = D3D11_USAGE_DEFAULT;
        TriangleVertDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        TriangleVertDesc.ByteWidth = sizeof(vertex) * ARRAY_COUNT(TriangleMeshVerts);
        
        D3D11_SUBRESOURCE_DATA TriangleVertData = { 0 };
        TriangleVertData.pSysMem = TriangleMeshVerts;
        
        Result = Renderer->Device->CreateBuffer(&TriangleVertDesc,
                                                &TriangleVertData,
                                                &Renderer->TriangleVBuffer );
        ASSERT(!FAILED(Result));
        
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
        Result = Renderer->Device->CreateBuffer(&QuadVertDesc,
                                                0,
                                                &Renderer->QuadVBuffer );
        
        D3D11_BUFFER_DESC QuadIndexDesc = { 0 };
        QuadIndexDesc.Usage          = D3D11_USAGE_DYNAMIC;
        QuadIndexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        QuadIndexDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
        QuadIndexDesc.ByteWidth      = sizeof(QuadMeshIndices);
        
        D3D11_SUBRESOURCE_DATA QuadIndexData = { 0 };
        QuadIndexData.pSysMem = QuadMeshIndices;
        
        Result = Renderer->Device->CreateBuffer(&QuadIndexDesc,
                                                &QuadIndexData,
                                                &Renderer->QuadIBuffer );
        ASSERT(!FAILED(Result));
        
        /// TEXT SQUARE
        
        u32 TextSpriteSize = sizeof(vertex) * ARRAY_COUNT(TextSpriteMeshVerts);
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
        Result = Renderer->Device->CreateBuffer(&TextSpriteVertDesc,
                                                0,
                                                &Renderer->TextSpriteVBuffer );
        ASSERT(!FAILED(Result));
        
        D3D11_BUFFER_DESC TextSpriteIndexDesc = { 0 };
        TextSpriteIndexDesc.Usage          = D3D11_USAGE_DEFAULT;
        TextSpriteIndexDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
        //TextSpriteIndexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        TextSpriteIndexDesc.ByteWidth = sizeof(u16) * (6 * CharLimit);
        
        D3D11_SUBRESOURCE_DATA TextSpriteIndexData = { 0 };
        TextSpriteIndexData.pSysMem = TextSpriteMeshIndices;
        Result = Renderer->Device->CreateBuffer(&TextSpriteIndexDesc,
                                                &TextSpriteIndexData,
                                                &Renderer->TextSpriteIBuffer );
        ASSERT(!FAILED(Result));
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
        Result = Renderer->Device->CreateBuffer(&GPUConstantsDesc,
                                                0,
                                                &Renderer->CBHigh);
        ASSERT(!FAILED(Result));
        
        GPUConstantsDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        GPUConstantsDesc.Usage          = D3D11_USAGE_DYNAMIC;
        GPUConstantsDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        GPUConstantsDesc.ByteWidth      = sizeof(gpu_const_low);
        GPUConstantsResource.pSysMem = &Renderer->ConstBufferLow;
        
        Result = Renderer->Device->CreateBuffer(&GPUConstantsDesc,
                                                &GPUConstantsResource,
                                                &Renderer->CBLow);
        ASSERT(!FAILED(Result));
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
           ARRAY_COUNT("..\\src\\default.hlsl"));
    
    char *ShaderPath = Renderer->CurrentShaderPath;
    
    
    FindFirstFileA(ShaderPath,
                   CurrentShaderFileInfo);
    
    HANDLE *ShaderCodeHandle = &Renderer->InUseShaderFileA;
    
    *ShaderCodeHandle = CreateFileA(ShaderPath,
                                    GENERIC_READ, 0, 0,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0);
    
    ASSERT(*ShaderCodeHandle);
    
    // NOTE(MIGUEL): Use an arena
    
    size_t ShaderFileSize = ((CurrentShaderFileInfo->nFileSizeHigh << 31) |
                             (CurrentShaderFileInfo->nFileSizeLow));
    
    ID3D11VertexShader *NewVertexShader;
    ID3D11PixelShader  *NewPixelShader;
    ASSERT(LoadShader(Renderer,
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
    gLineVLayout[2].AlignedByteOffset    = sizeof(v3f32);
    gLineVLayout[2].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
    gLineVLayout[2].InstanceDataStepRate = 1;
    
    
    WIN32_FIND_DATAA *LineShaderFileInfo = &Renderer->LineShaderFileInfo;
    
    memcpy(&Renderer->LineShaderPath,
           "..\\src\\lines.hlsl",
           ARRAY_COUNT("..\\src\\lines.hlsl"));
    
    char *LineShaderPath = Renderer->LineShaderPath;
    
    
    FindFirstFileA(LineShaderPath,
                   LineShaderFileInfo);
    
    HANDLE *LineShaderCodeHandle = &Renderer->InUseLineShaderFileA;
    
    *LineShaderCodeHandle = CreateFileA(LineShaderPath,
                                        GENERIC_READ, 0, 0,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        0);
    
    ASSERT(*LineShaderCodeHandle);
    
    size_t LineShaderFileSize = ((LineShaderFileInfo->nFileSizeHigh << 31) |
                                 (LineShaderFileInfo->nFileSizeLow));
    
    ID3D11VertexShader *NewLineVShader;
    ID3D11PixelShader  *NewLinePShader;
    ASSERT(LoadShader(Renderer,
                      &NewLineVShader,
                      &NewLinePShader,
                      &Renderer->LineInputLayout,
                      gLineVLayout, gLineVLayoutCount,
                      *LineShaderCodeHandle,
                      LineShaderFileSize,
                      AssetLoadingArena));
    
    Renderer->LineVShader = NewLineVShader;
    Renderer->LinePShader = NewLinePShader;
    
    return;
}

void D3D11HotLoadShader(renderer *Renderer,
                        ID3D11InputLayout  **InputLayout,
                        D3D11_INPUT_ELEMENT_DESC *ElemDesc,
                        u32 ElemDescCount,
                        memory_arena *ShaderLoadingArena)
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
        
        ID3D11VertexShader *NewVertexShader = nullptr;
        ID3D11PixelShader  *NewPixelShader  = nullptr;
        
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
                Renderer->VertexShader->Release();
                Renderer->PixelShader->Release();
                
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
                
                Renderer->VertexShader->Release();
                Renderer->PixelShader->Release();
                
                
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
                             memory_arena *ShaderLoadingArena)
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
        
        ID3D11VertexShader *NewVertexShader = nullptr;
        ID3D11PixelShader  *NewPixelShader  = nullptr;
        
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
                
                Renderer->LineVShader->Release();
                Renderer->LinePShader->Release();
                
                
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
                
                Renderer->LineVShader->Release();
                Renderer->LinePShader->Release();
                
                
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

void PhysicsSim(HWND Window, app_memory *AppMemory)
{
    
    memory_arena AssetLoadingArena;
    
    MemoryArenaInit(&AssetLoadingArena,
                    AppMemory->TransientStorageSize,
                    AppMemory->TransientStorage);
    
    
    D3D11LoadResources(&g_Renderer, &AssetLoadingArena);
    
    MemoryArenaDiscard(&AssetLoadingArena);
    
    g_Renderer.SmileyTex = LoadBitmap("..\\res\\frown.bmp");
    g_Renderer.TextTex   = LoadBitmap("..\\res\\text.bmp");
    
    D3D11InitTextureMapping(&g_Renderer,
                            &g_Renderer.SmileyTexView,
                            &g_Renderer.SmileySamplerState,
                            &g_Renderer.SmileyTex);
    D3D11InitTextureMapping(&g_Renderer,
                            &g_Renderer.TextTexView,
                            &g_Renderer.TextSamplerState,
                            &g_Renderer.TextTex);
    
    // NOTE(MIGUEL): Passive transformation is a transform that changes the coordinate
    //               system. (World moving around you when walking to simulate you looking around)
    //               Active transformation does not change the coordinate system instead
    //               it changes the vectors in the coordinate system. (This moving in the enviorment.)
    //               Both can be used
    
    
    win32_GetExeFileName(&g_winstate);
    
    char SimCodeDLLFullPathSource[WIN32_STATE_FILE_NAME_COUNT];
    win32_BuildExePathFileName(&g_winstate, "physics_sim_blah.dll",
                               sizeof(SimCodeDLLFullPathSource), SimCodeDLLFullPathSource);
    
    char SimCodeDLLFullPathTemp  [WIN32_STATE_FILE_NAME_COUNT];
    win32_BuildExePathFileName(&g_winstate, "physics_sim_blah_temp.dll",
                               sizeof(SimCodeDLLFullPathTemp), SimCodeDLLFullPathTemp);
    
    char SimCodeDLLFullPathLock  [WIN32_STATE_FILE_NAME_COUNT];
    win32_BuildExePathFileName(&g_winstate, "lock.tmp",
                               sizeof(SimCodeDLLFullPathLock), SimCodeDLLFullPathLock);
    
    
    app_input Input;
    
    
    LARGE_INTEGER TickFrequency;
    LARGE_INTEGER WorkStartTick;
    LARGE_INTEGER WorkEndTick;
    LARGE_INTEGER WorkTickDelta;
    LARGE_INTEGER MicrosElapsedWorking;
    //f32 TargetFPS = 60.0f;
    u64 TargetMicrosPerFrame = 16666;
    
    QueryPerformanceFrequency(&TickFrequency);
    
    win32_sim_code SimCode = { 0 };
    
    //renderer *Renderer = &g_Renderer;
    
    
    while (g_Running)
    {
        // NOTE(MIGUEL): Start Timer
        QueryPerformanceCounter  (&WorkStartTick);
        
        ProcessPendingMessages(&Input);
        
        RECT WindowDim;
        GetClientRect(Window, &WindowDim);
        g_WindowDim.Width  = WindowDim.right  - WindowDim.left;
        g_WindowDim.Height = WindowDim.bottom - WindowDim.top;
        
        g_WindowResized = true;
        
        
        if(!g_Pause)
        {
            memory_arena ShaderLoadingArena;
            
            MemoryArenaInit(&ShaderLoadingArena,
                            AppMemory->TransientStorageSize,
                            AppMemory->TransientStorage);
            
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
            
            MemoryArenaDiscard(&ShaderLoadingArena);
            
            FILETIME NewDLLWriteTime = win32_GetLastWriteTime(SimCodeDLLFullPathSource);
            {
                if(CompareFileTime(&NewDLLWriteTime, &SimCode.DLLLastWriteTime))
                {
                    win32_HotUnloadSimCode(&SimCode);
                    SimCode = win32_HotLoadSimCode(SimCodeDLLFullPathSource,
                                                   SimCodeDLLFullPathTemp,
                                                   SimCodeDLLFullPathLock);
                }
            }
            
            RenderBufferInit(&g_Renderer.RenderBuffer);
            
            if(SimCode.Update)
            {
                SimCode.Update(AppMemory, &g_Renderer.RenderBuffer);
            }
            
            
            
            Render(&g_Renderer, AppMemory);
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
            
            u64 TimeToSleep = (TargetMicrosPerFrame -
                               MicrosElapsedWorking.QuadPart +
                               MicrosElapsedWaiting.QuadPart) / 1000;
            
            Sleep(TimeToSleep);
            
            QueryPerformanceCounter  (&WaitEndTick);
            WaitTickDelta.QuadPart         = WaitEndTick.QuadPart - WaitStartTick.QuadPart;
            WaitTickDelta.QuadPart         = WaitTickDelta.QuadPart * 1000000;
            MicrosElapsedWaiting.QuadPart += (WaitTickDelta.QuadPart / TickFrequency.QuadPart);
        }
        
        // NOTE(MIGUEL): Fuck this is sloppy
        app_state *AppState = (app_state *)AppMemory->PermanentStorage;
        AppState->DeltaTimeMS  = (f32)((MicrosElapsedWorking.QuadPart + MicrosElapsedWaiting.QuadPart) / 1000);
        AppState->Time        += AppState->DeltaTimeMS;
        
    }
    
    return;
}

#if ALLOW_CRT
void WinMain()
#else
void WinMainCRTStartup()
#endif
{
    HWND Window = CreateOutputWindow();
    
    app_memory AppMemory = { 0 };
    
    //ASSERT(TestMathLib());
    
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
    
    // NOTE(MIGUEL): Device is created for processing rasterization
    if(D3D11Init(Window, &g_Renderer))
    {
        PhysicsSim(Window, &AppMemory);
    }
    
    
    D3D11Release(&g_Renderer);
    
    ExitProcess(0);
    
    return;
}

//~ CRT stuff
extern "C" int _fltused = 0x9875;

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

