cbuffer ChangeOnResize : register(b0)
{
  row_major matrix  UProj;
  row_major matrix  UView;
  float2            URes;
};

cbuffer ChangesEveryFrame : register(b1)
{
  //Chunk[0-4] 64 Bytes
  row_major matrix UWorld; 
  //Chunk[5] 16 Bytes
  float4           UColor;
  //Chunk[6] 16 Bytes
  float3           UPixelPos;
  float            UTime;
  //Chunk[7] 16 Bytes
  float            UWidth;
  uint             UJoinType;
  uint             UIsTextured;
};

struct VSInput
{
  float3 Pos   : POSITION;
  float4 Color : COLOR;
  float2 Tex   : TEXCOORD;
};

struct PSInput
{
  float4 Coord : SV_POSITION;
  float4 Color : COLOR;
  float2 Tex   : TEXCOORD;
};

//~ VERTEX SHADER

PSInput VS_Main(VSInput Vertex)
{
  PSInput VShaderOut = (PSInput)0;
  row_major matrix Transform = mul(UProj, UWorld);
  float4 NewPos = mul(Transform, float4(Vertex.Pos, 1.0));
  VShaderOut.Coord = NewPos;
  VShaderOut.Color = Vertex.Color;
  VShaderOut.Color = float4(1.0f, 1.3f, 0.3f, 1.0f);
  VShaderOut.Tex   = Vertex.Tex;
  return VShaderOut;
}

//~ PIXEL SHADER

Texture2D    Tex;
SamplerState Sampler;

float2 Noise(float2 uv)
{
  return 132.0*sin(dot(float2(0.1234, 0.24050), uv)*430940.0);
}

#define sf(x)     smoothstep( .1, .9 -step(.4,abs(x-.5)) , x-step(.9,x) )
#define sfrac(x) sf(frac(x))
#define smod(x,n) (sfrac((x)/(n))*(n))

float4 PS_Main(PSInput Frag) : SV_TARGET
{
  float uvdivs = 100.0;
  float2 uv = Frag.Coord.xy/URes.xy;
  uv -= -1.0;
  uv *= uvdivs;
  //uv += 30.0; 
  //uv.x += sin(uv.y)*0.3;
  //uv.y += sin(uv.x);
  //float2 Pos = UPixelPos.xy/Resolution.xy;
  
  //float Ring = smoothstep(0.0085, 0.009, length(uv-0.2));
  
  //uv*= 0.0f;
  //UV.x *=2.0f;
  //UV.x -= 1.0f;
  //UV.x = abs(UV.x);
  //float Line = -UV.x+1.0;
  float4 Color; 
  //Color = float4(uv.x, uv.y, Ring, Ring);
  float pw = 1.0/URes.y; //pixel width
  float2 w = float2(uvdivs/(4.0*URes.x), uvdivs/(4.0*URes.y));
  float2 a = float2(w.x - pw, w.y + pw);
  float2 b = float2(w.x - pw, w.y + pw);
  //abs(fract(x+.25)-.5)-.25
  float2 v = 1.-smoothstep(a, b, smod(uv+0.11, 1.0));
  //float h = smoothstep(a, b, frac(uv.y));
  float4 TexSample = float4(Tex.Sample(Sampler, Frag.Tex).r,
                            Tex.Sample(Sampler, Frag.Tex).r,
                            Tex.Sample(Sampler, Frag.Tex).r,
                            Tex.Sample(Sampler, Frag.Tex).r);
  if(UIsTextured == 1)
  {
    Color = TexSample;
  }
  else
  {
    //Color = lerp(float4(.10, .122,.18,1.), float4(v.x+v.y, v.x+v.y, v.x+v.y, 1.), 0.5f);
    Color = float4(0., 1., .0, 1.);
    Color = UColor;
  }
  Color.w = 0.0f;
  return Color;
  
}