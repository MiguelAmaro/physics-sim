
//~ C STRING
u64 CStrGetLength(const char *String, b32 IncludeNull)
{
  u64 Result = 0;
  while(String[Result] && Result<U32MAX) { Result++; }
  Result += IncludeNull?1:0;
  return Result;
}

static b32 CStrIsEqual(const char *a, const char *b)
{
  b32 Result = 1;
  size_t Length = CStrGetLength(a, 1);
  Assert(Length<3000);
  while((*a++==*b++) && (0<Length))
  {Length--;}
  Result = Length==0;
  
  return Result;
  
}
//~ CONNICAL STRINGS
str8 Str8Base(u8 *String, u64 Length)
{
  str8 Result = {0};
  Assert(Length<=U32MAX);
  Result.Data = String;
  Result.Length = (u32)Length;
  return Result;
}
str8 Str8FromCStr(char *String)
{
  str8 Result = {0};
  Result.Data = (u8 *)String;
  Result.Length = CStrGetLength(String, 0);
  return Result;
}
str8 Str8FromArena(arena *Arena, u64 Length)
{
  str8 Result = {0};
  Result.Length = Length;
  Result.Data = ArenaPushArray(Arena, Length, u8);
  return Result;
}
str8 Str8FromArenaFormat(arena *Arena, char const * Format, ...)
{
  va_list Args; va_start(Args, Format);
  str8 Result = Str8FromArena(Arena, stbsp_vsnprintf(NULL, 0, Format, Args));
  stbsp_vsnprintf((char *)Result.Data, SafeTruncateu64(Result.Length), Format, Args);
  va_end(Args);
  return Result;
}
b32 Str8IsEqual(str8 a, str8 b)
{
  b32 Result = 1;
  if(a.Length != b.Length) return 0;
  u64 Index = a.Length;
  while((Index>0) && (a.Data[Index-1]==b.Data[Index-1])) { Index--; }
  Result = (Index==0);
  return Result;
}
str8 Str8Concat(str8 a, str8 b, arena *Arena)
{
  str8 Result;
  u64 Length = a.Length + b.Length;
  u8 *Data = ArenaPushArray(Arena, Length, u8);
  MemoryCopy(a.Data,a.Length,Data,a.Length);
  MemoryCopy(b.Data,b.Length,Data+a.Length,b.Length);
  Result = Str8(Data, Length);
  return Result;
}
void S8Concat(u64 SourceACount, char *SourceA,
              u64 SourceBCount, char *SourceB,
              u64 DestCount   , char *Dest    )
{
  //TODO(MIGUEL): Dest bounds checking!
  
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

//
//str8 Str8InsertAt(char Char, const char *StringA, const char *StringB, arena Arena)
//{
//str8 Result = {0};
//Result.Data = (u8 *)String;
//Result.Size = CStrGetSize(String, 0);
//return Result;
//}
//

//~ STRING 32

str32 Str32(u32 *Data, u32 Length)
{
  str32 Result = {0};
  Result.Data = Data;
  Result.Length = Length;
  return Result;
}
//
//str32 Str32FromArena(arena Arena)
//{
//str32 Result = {0};
//Result.Data = Arena.Base;
//Result.Size = (u32)(Arena.Size/sizeof(u32));
//return Result;
//}
//
