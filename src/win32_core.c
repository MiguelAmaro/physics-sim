#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#define WIN32_STATE_FILE_NAME_COUNT (MAX_PATH)

static DWORD Win32ThreadContextId = 0;

//~ UTILS
fn void OSGenEntropy(void *Data, u64 Size)
{
  HCRYPTPROV Provider = 0;
  CryptAcquireContextA(&Provider, 0, 0, 0, CRYPT_VERIFYCONTEXT);
  CryptGenRandom(Provider, (u32)Size, (BYTE *)&Data);
  CryptReleaseContext(Provider, 0);
  return;
}
//~ MEMORY
fn void *OSMemoryReserve (u64 Size)
{
  void *Result = VirtualAlloc(0, Size, MEM_RESERVE, PAGE_READWRITE);
  return Result;
}
fn void  OSMemoryCommit  (void *Pointer, u64 Size)
{
  VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE);
  return;
}
fn void  OSMemoryDecommit(void *Pointer, u64 Size)
{
  VirtualAlloc(Pointer, Size, MEM_DECOMMIT, PAGE_READWRITE);
  return;
}
fn void  OSMemoryRelease (void *Pointer, u64 Size)
{
  VirtualFree(Pointer, 0, MEM_RELEASE);
  return;
}
fn void *OSMemoryAlloc(u64 Size)
{
  void *Result = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  Assert(Result);
  return Result;
}
//~ PROCESS SPECIFIC
fn void OSProcessKill(void)
{
  ExitProcess(0);
  return;
}
fn str8 OSProcessGetWorkDir(arena *Arena)
{
  //NOTE(MIGUEL): Maybe allocationg an extra byte
  u32  Size = GetCurrentDirectory(0, NULL);
  u8  *Data = ArenaPushArray(Arena, Size, u8);
  GetCurrentDirectoryA(Size, (LPSTR)Data);
  str8 Result = Str8(Data, Size-1);
  return Result;
}
//~ FILE SYSTEM
fn b32 OSFileExists(str8 FileName)
{
  // NOTE(MIGUEL): Can also get file attributes
  WIN32_FILE_ATTRIBUTE_DATA Ignored;
  b32 Result = GetFileAttributesEx((LPCSTR)FileName.Data,
                                   GetFileExInfoStandard,
                                   &Ignored);
  return Result;
}
fn datetime OSFileLastWriteTime(str8 FileName)
{
  /* Maybe usefull code
  WIN32_FIND_DATAA *LineShaderFileInfo = &Renderer->LineShaderFileInfo;
  FindFirstFileA(LineShaderPath,
                 LineShaderFileInfo);
  */
  datetime Result = {0};
  FILETIME   FileTime = {0};
  SYSTEMTIME SysTime  = {0};
  WIN32_FILE_ATTRIBUTE_DATA FileInfo;
  Assert(GetFileAttributesEx((LPCSTR)FileName.Data, GetFileExInfoStandard, &FileInfo));
  FileTime = FileInfo.ftLastWriteTime;
  FileTimeToSystemTime(&FileTime, &SysTime);
  Result.year = (s16)SysTime.wYear;
  Result.mon  = (u8) SysTime.wMonth;
  //Result.day  = SysTime.wDayOfWeek;
  Result.day  = (u8)SysTime.wDay;
  Result.hour = (u8)SysTime.wHour;
  Result.min  = (u8)SysTime.wMinute;
  Result.sec  = (u8)SysTime.wSecond;
  Result.ms   = (u8)SysTime.wMilliseconds;
  return Result;
}
fn void OSGetExeFileName(os_state *State)
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
fn u64 OSFileGetSize(str8 Path)
{
  HANDLE Handle = CreateFileA((LPCSTR)Path.Data, GENERIC_READ, FILE_SHARE_READ, 0,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  LARGE_INTEGER Whatever = { 0 };
  u64 Result = 0;
  b32 Status = GetFileSizeEx(Handle, &Whatever);
  Result = Whatever.QuadPart;
  Assert(Status != 0);
  CloseHandle(Handle);
  return Result;
}
fn str8 OSFileRead(str8 Path, arena *Arena)
{
  str8 Result = {0};
  HANDLE File = CreateFileA((LPCSTR)Path.Data, GENERIC_READ, FILE_SHARE_READ,
                            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if(File)
  {
    u64 ExpectedSize = OSFileGetSize(Path); Assert(ExpectedSize<U32MAX);
    u64 BytesRead    = 0;
    Result = Str8(Arena, ExpectedSize);
    ReadFile(File, Result.Data, (u32)ExpectedSize, (LPDWORD)&BytesRead, NULL);
    Assert(BytesRead == ExpectedSize);
    Result.Length = BytesRead;
  }
  CloseHandle(File);
  return Result;
}
//~ STATISTICS

//~ LOGGING METHODS
fn u32 OSConsoleLogF(arena Arena, char *Format, ...)
{
  va_list ArgList;
  va_start(ArgList, Format);
  str8 String = Str8FromArena(&Arena, stbsp_vsnprintf(NULL, 0, Format, ArgList) + 1);
  stbsp_vsnprintf((char *)String.Data, (u32)String.Length, Format, ArgList);
  u32 WriteCount = 0;
  WriteConsoleA((HANDLE)gState->Console, String.Data, (u32)String.Length, (LPDWORD)&WriteCount, NULL);
  return WriteCount;
}
fn u32 OSConsoleLogStrLit(char *StringLit)
{
  str8 String = Str8((u8 *)StringLit, CStrGetLength(StringLit, 0));
  u32 WriteCount = 0;
  WriteConsoleA((HANDLE)gState->Console, String.Data, (u32)String.Length, (LPDWORD)&WriteCount, NULL);
  return WriteCount;
}
fn void OSConsoleCreate(void)
{
  b32 Status = AllocConsole();
  Assert(Status != 0);
  gState->Console =
    (u64)CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ,
                                   NULL,
                                   CONSOLE_TEXTMODE_BUFFER,
                                   NULL);
  Status = SetConsoleActiveScreenBuffer((HANDLE)gState->Console);
  return;
}
fn void OSFatalError(const char* Message)
{
  MessageBoxA(NULL, Message, "Error", MB_ICONEXCLAMATION);
  ExitProcess(0);
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
//~ MODULES
os_module OSModuleLoad(str8 Path)
{
  StrIsNullTerminated(Path);
  HMODULE Module = LoadLibraryA((const char *)Path.Data);
  return (os_module)Module;
}
void OSModuleUnload(os_module Module)
{
  FreeLibrary((HMODULE)Module);
  return;
}
voidproc *OSModuleGetProc(os_module Module, str8 ProcName)
{
  StrIsNullTerminated(ProcName);
  voidproc *Proc = (voidproc *)GetProcAddress((HMODULE)Module, (LPCSTR)ProcName.Data);
  return Proc;
}
//~ EVENTS
v2f OSEventsGetCursorPos(app_input *Input, HWND Window)
{
  POINT CursorPos;
  GetCursorPos(&CursorPos);
  ScreenToClient(Window, &CursorPos);
  v2f Result = V2f((f32)CursorPos.x ,(f32)CursorPos.y);
  return Result;
}
fn void OSEventsInit(os_events *Events)
{
  Events->Events = NULL;
  Events->Count = 0;
  Events->Arena = ArenaInit(NULL,
                            128*sizeof(os_event),
                            ArenaPushArray(&gState->Arena, 128, os_event));
  MemorySet(0, Events->Arena.Base, Events->Arena.Size);
  return;
}
fn os_event *OSEventsPush(os_events *Events, os_event Event)
{
  os_event *NewEvent = ArenaPushType(&Events->Arena, os_event);
  Assert(NewEvent);
  *NewEvent = Event;
  if(Events->Count == 0)
  {
    Events->Events = NewEvent;
    NewEvent->Next  = NULL;
    NewEvent->First = NewEvent;
    NewEvent->Last  = NewEvent;
  }
  else
  {
    os_event *FirstEvent = Events->Events->First;
    os_event *LastEvent  = Events->Events->Last;
    NewEvent->Next  = NULL;
    NewEvent->First = FirstEvent;
    LastEvent->Next = NewEvent;
  }
  Events->Count++;
  return NewEvent;
}
fn void OSEventsProcessKeyPress(os_event *Event, b32 IsDown)
{
  if(Event->EndedDown != IsDown)
  {
    Event->EndedDown = IsDown;
    ++Event->HalfTransitionCount;
  }
  return;
}
fn void OSEventsReset(os_events *Events)
{
  ArenaReset(&Events->Arena);
  Events->Count = 0;
  os_event Event = { .Kind = Event_Null };
  OSEventsPush(Events, Event);
  return;
}
fn LRESULT CALLBACK OSEventsDefaultHandler(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
  LRESULT Result = 0;
  Result = DefWindowProcW(Window, Message, WParam, LParam);
  return Result;
}
fn void OSEventsConsume(os_events *Events)
{
  MSG Message = {0};
  while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
  {
    switch(Message.message)
    {
      case WM_QUIT: { gState->Running = 0; }  break;
      case WM_SYSKEYUP:
      case WM_SYSKEYDOWN:
      case WM_KEYUP:
      case WM_KEYDOWN:
      {
        u32 VKCode          = (u32)Message.wParam;
        u32 KeyWasDown      = ((Message.lParam & (1 << 30)) != 0);
        u32 KeyIsDown       = ((Message.lParam & (1 << 31)) == 0);
        if(KeyWasDown != KeyIsDown)
        {
          if(VKCode >= 'A' && VKCode <= 'Z')
          { 
            os_event Event = {0};
            Event.Key = Key_A + (VKCode - 'A');
            OSEventsProcessKeyPress(&Event, KeyIsDown);
            OSEventsPush(Events, Event);
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
//~ WINDOW
fn void OSWindowGetNewSize(void)
{
  os_state *State = gState;
  RECT Rect;
  Assert(State->Window != 0);
  GetClientRect((HANDLE)State->Window, &Rect);
  State->WindowDim.x = Rect.right - Rect.left;
  State->WindowDim.y = Rect.bottom - Rect.top;
  return;
}
fn void OSWindowCreate(void)
{
  os_state *State = gState;
  WNDCLASSEXW WindowClass = {0};
  WindowClass.cbSize        = sizeof(WindowClass);
  WindowClass.lpfnWndProc   = &OSEventsDefaultHandler;
  WindowClass.style         = CS_HREDRAW | CS_VREDRAW;
  WindowClass.hInstance     = GetModuleHandleW(NULL);
  WindowClass.hIcon         = LoadIconA(NULL, IDI_APPLICATION);
  WindowClass.hCursor       = LoadCursorA(NULL, IDC_ARROW);
  WindowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  WindowClass.lpszClassName = L"UI SandBox";
  u64 Result = 0;
  if(RegisterClassExW(&WindowClass))
  {
    DWORD ExStyle = 0; //WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
    RECT WindowDim = { 0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT };
    AdjustWindowRect(&WindowDim, WS_OVERLAPPEDWINDOW, 0);
    HWND Window = CreateWindowExW(ExStyle, WindowClass.lpszClassName,
                                  L"UI SandBox",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  DEFAULT_WINDOW_COORDX, DEFAULT_WINDOW_COORDY,
                                  (WindowDim.right  - WindowDim.left),
                                  (WindowDim.bottom - WindowDim.top),
                                  0, 0, WindowClass.hInstance, 0);
    Result = (u64)Window;
  }
  State->Window = (u64)Result;
  OSWindowGetNewSize();
  return;
}
//~ THREAD CONTEXT
fn void OSThreadCtxSet(void *Ptr)
{
  TlsSetValue(Win32ThreadContextId, Ptr);
  return;
}
fn void *OSThreadCtxGet(void)
{
  void *Result = TlsGetValue(Win32ThreadContextId);
  return Result;
}
fn arena *OSThreadCtxGetScratch(os_thread_ctx *Ctx, arena **Conflicts, u32 ConflictCount)
{
  arena *Result = NULL;
  arena *Scratch = Ctx->ScratchPool;
  for(u32 Idx=0; Idx< ArrayCount(Ctx->ScratchPool); Idx++, Scratch++)
  {
    b32 IsNotConflict = 1;
    arena **Conflict = Conflicts;
    for(u32 j=0; j<ConflictCount; j++, Conflict++)
    {
      if(Scratch == *Conflict) { IsNotConflict = 0; break; }
    }
    if(IsNotConflict) { Result = Scratch; break; }
  }
  return Result;
}
fn void OSThreadCtxInit(os_thread_ctx *Ctx, void *Memory, u64 MemorySize)
{
  Ctx->Memory = Memory;
  arena *Scratch = Ctx->ScratchPool;
  for(u32 Idx=0; Idx< ArrayCount(Ctx->ScratchPool); Idx++, Scratch++)
  {
    *Scratch = ArenaInit(NULL, MemorySize, Memory); 
  }
  return;
}
//~ STATE
fn void OSStateInit(os_state **OSState)
{
  os_state State = 
  {
    .Permanent     = OSMemoryAlloc(Gigabytes(1)),
    .PermanentSize = Gigabytes(1),
    .Transient     = OSMemoryAlloc(Gigabytes(2)),
    .TransientSize = Gigabytes(2),
    .Pool          = OSMemoryAlloc(Gigabytes(1)),
    .PoolSize      = Gigabytes(1),
    .WindowDim     = V2s(0,0),
    .Running       = 1,
    .Console       = 0, ///What exactly is a HANDLE in windows?
    .Window        = 0,
  };
  State.Arena   = ArenaInit(NULL, State.PermanentSize, State.Permanent);
  State.WorkDir = OSProcessGetWorkDir(&State.Arena);
  *OSState    = ArenaPushType(&State.Arena, os_state);
  **OSState   = State;
  OSGetExeFileName(*OSState);
  Win32ThreadContextId = TlsAlloc();
  return;
}
//~ TIME

typedef struct time_measure time_measure;
struct time_measure
{
  f64 MSDelta;
  f64 MSElapsed;
  u64 TickFrequency;
  u64 WorkStartTick;
  u64 WorkEndTick;
  u64 WorkTickDelta;
  f64 MicrosElapsedWorking;
  f64 TargetMicrosPerFrame;
  f64 TicksToMicros;
};
time_measure OSTimerInit(void)
{
  time_measure Result = {0};
  Result.MSDelta   = 0.0;
  Result.MSElapsed = 0.0;
  Result.TickFrequency = 0;
  Result.WorkStartTick = 0;
  Result.WorkEndTick = 0;
  Result.WorkTickDelta = 0;
  Result.MicrosElapsedWorking = 0.0;
  Result.TargetMicrosPerFrame = 16666.0;
  Result.TicksToMicros = 1000000.0/(f64)Result.TickFrequency;
  QueryPerformanceFrequency((LARGE_INTEGER *)&Result.TickFrequency);
  return Result;
}
void OSTimerStart(void)
{
  return;
}
f64 OSTimeMeasureElapsed(time_measure *Time)
{
  QueryPerformanceCounter((LARGE_INTEGER *)&Time->WorkEndTick);
  Time->WorkTickDelta  = Time->WorkEndTick - Time->WorkStartTick;
  Time->MicrosElapsedWorking = (f64)Time->WorkTickDelta*Time->TicksToMicros;
  u64 IdleTickDelta = 0;
  u64 IdleStartTick = Time->WorkEndTick;
  u64 IdleEndTick = 0;
  f64 MicrosElapsedIdle = 0.0;
  while((Time->MicrosElapsedWorking+MicrosElapsedIdle)<Time->TargetMicrosPerFrame)
  {
    QueryPerformanceCounter((LARGE_INTEGER *)&IdleEndTick);
    IdleTickDelta = IdleEndTick-IdleStartTick;
    MicrosElapsedIdle = (f64)IdleTickDelta*Time->TicksToMicros;
  }
  f64 FrameTimeMS = (Time->MicrosElapsedWorking+MicrosElapsedIdle)/1000.0;
  Time->MSDelta    = FrameTimeMS;
  Time->MSElapsed += FrameTimeMS;
  return Time->MSDelta;
}