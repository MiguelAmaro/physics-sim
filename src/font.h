#ifndef FONT_H
#define FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "images.h"

global FT_Library FreeType;
global FT_Face    Face;

fn void FontInit(const char* FontFileName, u32 Point)
{
  Assert(!FT_Init_FreeType(&FreeType) &&
         "FreeType Error: Could not init FreeType Library");
  Assert(!FT_New_Face(FreeType, FontFileName, 0, &Face) &&
         "FreeType Error: Could not load Font");
  FT_Set_Pixel_Sizes(Face, 0, Point);
  return;
}
fn glyph_metrics FontGetGlyph(u32 Codepoint, arena *Arena)
{
  glyph_metrics Result = {0};
  FT_Glyph_Metrics *GlyphMetrics = &Face->glyph->metrics;
  FT_Bitmap        *GlyphBitmap  = &Face->glyph->bitmap;
  if(FT_Load_Char(Face, Codepoint, FT_LOAD_RENDER))
  {
    ConsoleLog("FreeType Error: Could not load Glyph");
    return Result;
  }
  f32 UnitConversion = 1.0f/64.0f;
  Result.Dim     = Scale(V2f((f32)GlyphMetrics->width, (f32)GlyphMetrics->height), UnitConversion);
  Result.Bearing = Scale(V2f((f32)GlyphMetrics->horiBearingX, (f32)GlyphMetrics->horiBearingY), UnitConversion);
  Result.Advance = (f32)GlyphMetrics->horiAdvance*UnitConversion;
  u8  *GlyphBitmapData = GlyphBitmap->buffer;
  u32  GlyphBitmapSize = (u32)(Result.Dim.x * Result.Dim.y);
  if(GlyphBitmapSize>0)
  {
    bitmapdata Data = {0};
    Data.Width  = (u32)Result.Dim.x;
    Data.Height = (u32)Result.Dim.y;
    Data.Pixels = (u8 *)ArenaPushArray(Arena, GlyphBitmapSize, u8);
    Data.BytesPerPixel = sizeof(u8);
    MemoryCopy(GlyphBitmapData, GlyphBitmapSize, Data.Pixels, GlyphBitmapSize);
    Result.BitmapData = Data;
  }
  return Result;
}
fn void D3D11LoadTextGlyphs(renderer *Renderer, arena *AssetLoadingArena)
{
  u32 AsciiStart = 32;
  u32 AsciiEnd   = 126;
  for(u32 CharCode = AsciiStart; CharCode < AsciiEnd; CharCode++)
  {
    glyph_metrics Metrics = FontGetGlyph(CharCode, AssetLoadingArena);
    Renderer->GlyphMetrics[CharCode] = Metrics;
    ConsoleLog(*AssetLoadingArena,
               "Glyph Metrics: \"%c\" | "
               "H: %d  W: %d  | "
               "HoriAdv: %d   | "
               "BearingX: %d  | "
               "Bearing Y: %d | "
               "\n",
               (char)CharCode,
               (u32)Metrics.Dim.x, (u32)Metrics.Dim.y,
               (u32)Metrics.Advance,
               (u32)Metrics.Bearing.x, (u32)Metrics.Bearing.y);
  }
  return;
}

#endif //FONT_H
