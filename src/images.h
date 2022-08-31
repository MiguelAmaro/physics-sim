#ifndef IMAGES_H
#define IMAGES_H

typedef struct bitmapdata bitmapdata;
struct bitmapdata
{
  u32  Width;
  u32  Height;
  u8  *Pixels;
  u32  BytesPerPixel;
};

#pragma pack(push, 1)
typedef struct bitmapheader bitmapheader;
struct bitmapheader
{
  u16 FileType;
  u32 FileSize;
  u16 reserved_1;
  u16 reserved_2;
  u32 BitmapOffset;
  u32 Size;
  s32 Width;
  s32 Height;
  u16 Planes;
  u16 BitsPerPixel;
  u32 Compression;
  u32 BitmapSize;
  s32 HRes;
  s32 VRes;
  u32 ColorsUsed;
  u32 ColorsImportant;
  
  u32 RedMask;
  u32 GreenMask;
  u32 BlueMask;
};
#pragma pack(pop)

typedef struct glyph_metrics glyph_metrics;
struct glyph_metrics
{
  // NOTE(MIGUEL): // Horizontl text layout
  v2f Dim;
  v2f Bearing; 
  f32 Advance;
  bitmapdata BitmapData; 
};

fn bitmapdata LoadBitmapData(const char *FileName)
{
  bitmapdata Result = { 0 };
  LARGE_INTEGER FileSize;
  HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ,
                                  0, OPEN_EXISTING, 0, 0);
  GetFileSizeEx(FileHandle, &FileSize);
  u32 FileSize32 = SafeTruncateu64(FileSize.QuadPart);
  void *Block = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  Assert(Block);
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
    Result.Pixels = (u8 *)Pixels;
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
      Assert(  RedShift.Found);
      Assert(GreenShift.Found);
      Assert( BlueShift.Found);
      Assert(AlphaShift.Found);
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
  return Result;
}

#endif //IMAGES_H
