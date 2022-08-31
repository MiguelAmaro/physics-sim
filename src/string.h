#ifndef STRING_H
#define STRING_H

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

typedef struct str8 str8;
struct str8
{
  u8  *Data;
  u64  Length;
};

typedef struct str16 str16;
struct str16
{
  u16 *Data;
  u64 Length;
};

typedef struct str32 str32;
struct str32
{
  u32 *Data;
  u64  Length;
};

typedef struct ucodepoint ucodepoint;
struct ucodepoint
{
  u32 CodePoint;
  u64 Length;
};

#define Str8(...) _Generic(ARG1(__VA_ARGS__),  \
u8 *   : Str8Lit,    \
char * : Str8FromCStr, \
arena *: Str8FromArena)(__VA_ARGS__)

//~ C STRINGS
u64 CStrGetLength(const char *String, b32 IncludeNull);
b32 CStrIsEqual(const char *a, const char *b);
b32 StrIsNullTerminated(str8 Str);
//~ 8 BIT STRINGS
str8 Str8Lit(u8 *String, u64 Size);
str8 Str8FromCStr(char *String);
str8 Str8FromArena(arena *Arena, u64 Size);
str8 Str8FromArenaFormat(arena *Arena, char const * Format, ...);
b32  Str8IsEqual(str8 a, str8 b);
str8 Str8Concat(str8 a, str8 b, arena *Arena);
void S8Concat(u64 SourceACount, char *SourceA,
              u64 SourceBCount, char *SourceB,
              u64 DestCount   , char *Dest    );
str8 Str8InsertAt(char Char, const char *StringA, const char *StringB, arena Arena);
//~ 32 BIT STRINGS
str32 Str32(u32 *Data, u32 Size);
str32 Str32FromArena(arena Arena);

#endif //STRING_H
