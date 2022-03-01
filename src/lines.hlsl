cbuffer ChangesOnResize : register(b0)
{
    row_major matrix  UProj;
    row_major matrix  UView;
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
    row_major matrix UWorld;
    float4           UColor;
    float            UTime;
    float            UWidth;
    uint             UJoinType;
};

struct VSInput
{
    float3 Pos    : POSITION;
    float3 PointA : INST_POINT_A;
    float3 PointB : INST_POINT_B;
};

struct PSInput
{
    float4 Coords : SV_POSITION;
};

//~ VERTEX SHADER

float2 BuildLineSegment(float3 Pos, float3 PointA, float3 PointB, float Width)
{
    float2 Result = {0.0,0.0};
    float2 BasisX = PointB.xy - PointA.xy;
    float2 NBX = {-BasisX.y, BasisX.x};
    float2 BasisY = normalize(NBX);
    Result = PointA.xy + BasisX * Pos.x + BasisY * Width * Pos.y;
    
    return Result;
}

float2 BuildRoundJoin(float3 Pos, float3 Point, float Width)
{
    float2 Result = {0.0,0.0};
    Result = Width * Pos.xy + Point.xy ;
    
    return Result;
}


PSInput VS_Main(VSInput Vert, uint instanceID : SV_InstanceID)
{
    PSInput VShaderOut = (PSInput)0;
    
    row_major matrix Transform = UProj;
    
    float ExtraW = 0.0;
    
    float4 NewPos = 0.0;
    if(UJoinType == 0)
    {
        float2 Point = BuildLineSegment(Vert.Pos, Vert.PointA, Vert.PointB, UWidth + ExtraW);
        NewPos = float4(Point.x, Point.y, 0.0f, 1.0f);
    }
    else if(UJoinType == 1)
    {
        float2 Point = BuildRoundJoin(Vert.Pos, Vert.PointB, UWidth + ExtraW);
        NewPos = float4(Point.x, Point.y, 0.0f, 1.0f);
    }
    
    VShaderOut.Coords = mul(UProj, NewPos);
    
    return VShaderOut;
}

//~ PIXEL SHADER

float4 PS_Main(PSInput Frag) : SV_TARGET
{
    float2 uv = Frag.Coords.xy/Resolution.xy;
    float T = UTime * 0.0008;
    
    //uv = float2(atan2(uv.x, uv.y), length(uv));
    
    uv.x *= 5.0;
    uv.y *= 8.0;
    uv += T;
    uv = floor(uv);
    
    float4 Color =
    {
        abs(sin(uv.x + T)),
        abs(cos(uv.y + T)),
        abs(cos(uv.y + T)), 1.0f
    };
    
    return Color * UColor;
}
