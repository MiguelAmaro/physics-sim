
cbuffer ChangesEveryFrame : register(b0)
{
    row_major matrix World;
    float             Time;
};
cbuffer NeverChanges : register(b1)
{
    row_major matrix  View;
};
cbuffer ChangesOnResize : register(b2)
{
    row_major matrix  Proj;
};

struct VSInput
{
    float4 Pos   : POSITION;
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
    
    row_major matrix Transform = mul(World, Proj);
    
    float4 NewPos = mul(Vertex.Pos, Transform);
    
    VertexShaderOutput.Color = Vertex.Color;
    VertexShaderOutput.Color = float4(1.0f, 0.3f, 0.3f, 1.0f);
    
    
    VertexShaderOutput.Pos   = NewPos;
    VertexShaderOutput.Tex = Vertex.Tex;
    
    return VertexShaderOutput;
}

//~ PIXEL SHADER

Texture2D    Tex;
SamplerState Sampler;

float4 PS_Main(PSInput Fragment) : SV_TARGET
{
    
    return Fragment.Color;
    
    //return Tex.Sample(Sampler, Fragment.Tex);
    
}