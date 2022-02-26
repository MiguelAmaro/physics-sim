cbuffer ChangesOnResize : register(b0)
{
    row_major matrix  Proj;
    row_major matrix  View;
    float2 Resolution;
};

/*
ENUM: JOINTYPE
NOTJOIN/NOJOIN/ISLINE=0
ROUND=1
MILTER=2
*/

cbuffer ChangesEveryFrame : register(b1)
{
    row_major matrix World;
    float4           UColor;
    float             Time;
    float  Width;
    uint   JoinType;
};

struct VSInput
{
    float4 Pos    : POSITION;
    float3 PointA : INST_POINT_A;
    float3 PointB : INST_POINT_B;
};

struct PSInput
{
    float4 Coords : SV_POSITION;
};

//~ VERTEX SHADER

float2 BuildLineSegment(float3 Pos, float3 PointA, float3 PointB)
{
    float2 Result = {0.0,0.0};
    float2 BasisX = PointB.xy - PointA.xy;
    float2 NBX = {-BasisX.y, BasisX.x};
    float2 BasisY = normalize(NBX);
    Result = PointA.xy + BasisX * Pos.x + BasisY * Width * Pos.y;
    
    return Result;
}

float2 BuildRoundJoin(float3 Pos, float3 Point)
{
    float2 Result = {0.0,0.0};
    Result = Width * Pos.xy + Point.xy ;
    
    return Result;
}


PSInput VS_Main(VSInput Vert, uint instanceID : SV_InstanceID)
{
    PSInput VShaderOut = (PSInput)0;
    
    row_major matrix Transform = Proj;
    
    float4 NewPos = 0.0;
    if(JoinType == 0)
    {
        float2 Point = BuildLineSegment(Vert.Pos.xyz, Vert.PointA, Vert.PointB);
        NewPos = float4(Point.x, Point.y, 0.0f, 1.0f);
    }
    else if(JoinType == 1)
    {
        float2 Point = BuildRoundJoin(Vert.Pos.xyz, Vert.PointB);
        NewPos = float4(Point.x, Point.y, 0.0f, 1.0f);
    }
    
    VShaderOut.Coords = mul(Proj, NewPos);
    
    return VShaderOut;
}

//~ PIXEL SHADER

float4 PS_Main(PSInput Frag) : SV_TARGET
{
    float2 uv = Frag.Coords.xy/Resolution.xy;
    
    float Radius = Width/2.0f;
    
    float T = Time * 0.001;
    float4 Color =
    {
        abs(sin(uv.x + T)),
        abs(cos(uv.y + T)),
        abs(cos(uv.y + T)), 1.0f
    };
    
    return Color * UColor;
}