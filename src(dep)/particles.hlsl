struct particle
{
  float3 Pos;
  float3 Vel;
  float  Time;
};

AppendStructuredBuffer<partcle> NewSimState : register(u0);
ConsumeStructuredBuffer<partcle> CurrSimState : register(u1);

cbuffer SimParams
{
  float4 TimeFactors;
  float4 EmitterLocation;
  float4 ConsumerLocation;
};

[numthreads(512, 1, 1)]

void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
  uint MyId = (DispatchThreadID.x +
               DispatchThreadID.y * 512 +
               DispatchThreadID.z * 512 * 512);
  
  if(MyId < NumParticles.x)
  {
    //
  }
}