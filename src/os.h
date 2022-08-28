#ifndef OS_H
#define OS_H


#define DEFAULT_WINDOW_COORDX  0
#define DEFAULT_WINDOW_COORDY  10
#define DEFAULT_WINDOW_WIDTH  800
#define DEFAULT_WINDOW_HEIGHT 800

typedef u64 os_window;
typedef u64 os_handle;

typedef struct os_state os_state;
struct os_state
{
  u64   Console;
  u64   Window;
  str8  WorkDir;
  v2s   WindowDim;
  b32   Running;
  u8   *Permanent;
  u64   PermanentSize;
  u8   *Transient;
  u64   TransientSize;
  u8   *Pool;
  u64   PoolSize;
  arena Arena;
};

typedef enum os_event_kind os_event_kind;
enum os_event_kind
{
  Event_Null,
  Event_KeyPress,
  Event_Cursor,
};

typedef enum os_key os_key;
enum os_key
{
  Key_A,
  Key_B,
  Key_C,
  Key_D,
  Key_E,
  Key_F,
  Key_G,
  Key_H,
  Key_I,
  Key_J,
  Key_L,
  Key_M,
  Key_N,
  Key_O,
  Key_P,
  Key_Q,
  Key_R,
  Key_S,
  Key_W,
  Key_X,
  Key_Y,
  Key_Z,
  Key_Count,
};

typedef struct os_event os_event;
struct os_event
{
  os_event *First;
  os_event *Next;
  os_event *Last;
  os_event_kind Kind;
  os_key Key;
  b32 EndedDown;
  u32 HalfTransitionCount;
  //mouse
};

typedef struct os_events os_events;
struct os_events
{
  os_event *Events;
  u32 Count;
  arena Arena;
};

os_state *gState;
os_events gEvents;

#define ConsoleLog(...) _Generic(ARG1(__VA_ARGS__), \
arena: OSConsoleLogF, \
char *: OSConsoleLogStrLit)(__VA_ARGS__)

fn void *OSMemoryReserve (u64 Size);
fn void  OSMemoryCommit  (void *Pointer, u64 Size);
fn void  OSMemoryDecommit(void *Pointer, u64 Size);
fn void  OSMemoryRelease (void *Pointer, u64 Size);
fn void  OSGenEntropy    (void *Data, u64 Size);

#endif //OS_H
