
cbuffer ChangesEveryFrame : register(b0)
{
  row_major matrix World;
  float4 Color;
  float             Time;
};
cbuffer NeverChanges : register(b1)
{
  row_major matrix  Proj;
  row_major matrix  View;
  float2 Resolution;
};

struct VSInput
{
  float3 Pos   : POSITION;
  float4 Color : COLOR;
  float2 Tex   : TEXCOORD;
};

struct PSInput
{
  float4 Pos   : SV_POSITION;
  float4 Color : COLOR;
  float2 Tex   : TEXCOORD;
};

//~ VERTEX SHADER

PSInput VS_Main(VSInput Vertex)
{
  PSInput VertexShaderOutput = (PSInput)0;
  
  row_major matrix Transform = mul(Proj, World);
  //Transform = mul(View, Transform);
  
  float4 NewPos = mul(Transform, float4(Vertex.Pos, 1.0));
  
  VertexShaderOutput.Color = Vertex.Color;
  VertexShaderOutput.Color = float4(1.0f, 1.3f, 0.3f, 1.0f);
  
  //NewPos += 1.0;
  
  VertexShaderOutput.Pos = NewPos;
  VertexShaderOutput.Tex = Vertex.Tex;
  
  return VertexShaderOutput;
}

//~ PIXEL SHADER

Texture2D    Tex;
SamplerState Sampler;

float4 PS_Main(PSInput Frag) : SV_TARGET
{
  float2 UV = Frag.Pos.xy/Resolution.xy;
  UV *= 0.0f;
  UV.x *=2.0f;
  UV.x -= 1.0f;
  UV.x = abs(UV.x);
  float Line = -UV.x+1.0;
  //float4 Color = float4(UV, 0.0f, 1.0f);
  float4 Color = float4(0.0f, 1.0f, 0.0f, smoothstep(Line, Line+0.00001f, UV.y));
  
  
  return Color;
  
  //return Tex.Sample(Sampler, Frag.Tex);
  
}