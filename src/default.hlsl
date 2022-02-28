
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
    
    return Frag.Color;
    
    //return Tex.Sample(Sampler, Fragment.Tex);
    
}