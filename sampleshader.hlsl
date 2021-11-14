
cbuffer ChangesPerFrame
{
    row_major matrix MVP;
    float            Time;
}

struct VSInput
{
    float4 Pos   : POSITION;
    float4 Color : COLOR;
    // float2 Tex   : TEXCOORD;
};

struct PSInput
{
    float4 Pos   : SV_POSITION;
    float4 Color : COLOR;
    //float2 Tex   : TEXCOORD;
};

//~ VERTEX SHADER

PSInput VS_Main(VSInput Vertex)
{
    PSInput VertexShaderOutput = (PSInput)0;
    
    //Vertex.Pos.x += cos(Time);
    //Vertex.Pos.y += sin(Time);
    //Vertex.Pos.z *= tan(Time);
    
    VertexShaderOutput.Pos   = mul(Vertex.Pos, MVP);
    VertexShaderOutput.Color = Vertex.Color;
    //VertexShaderOutput.Tex   = Vertex.Tex;
    
    
    
    return VertexShaderOutput;
}

//~ PIXEL SHADER

//Texture2D     Tex;
//SamplerState Sampler;

float4 PS_Main(PSInput Fragment) : SV_TARGET
{
    
    // return Tex.Sample(Sampler, Fragment.Tex);
    
    float4 Color;
    
    return Fragment.Color;
    
}
