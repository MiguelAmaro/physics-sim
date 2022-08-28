#include <wincrypt.h>
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
//~ LOGGING METHODS
fn u32 OSConsoleLogF(arena Arena, char *Format, ...)
{
  va_list ArgList;
  va_start(ArgList, Format);
  str8 String = Str8FromArena(&Arena, stbsp_vsprintf(NULL, Format, ArgList) + 1);
  stbsp_vsprintf((char *)String.Data, Format, ArgList);
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
//~ EVENTS
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
