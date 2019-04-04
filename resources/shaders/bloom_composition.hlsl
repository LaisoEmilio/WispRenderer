#include "hdr_util.hlsl"

Texture2D source_main : register(t0);
Texture2D source_bloom : register(t1);
RWTexture2D<float4> output : register(u0);
SamplerState linear_sampler : register(s0);
SamplerState point_sampler : register(s1);

cbuffer Bloomproperties : register(b0)
{
	int enable_bloom;
};

[numthreads(16, 16, 1)]
void main_cs(int3 dispatch_thread_id : SV_DispatchThreadID)
{
	float2 screen_size = float2(0.f, 0.f);
	output.GetDimensions(screen_size.x, screen_size.y);

	float2 screen_coord = int2(dispatch_thread_id.x, dispatch_thread_id.y);
	float2 texel_size = 1.0f / screen_size;

	float2 uv = (screen_coord + 0.5f) / screen_size;

	float3 finalcolor = float3(0, 0, 0);

	float gamma = 2.2;
	float exposure = 1;

	if (enable_bloom > 0)
	{
		finalcolor += source_bloom.SampleLevel(linear_sampler, uv, 0).rgb;
	}
	finalcolor += source_main.SampleLevel(point_sampler, uv, 0).rgb;

	finalcolor = linearToneMapping(finalcolor, exposure, gamma);
	output[int2(dispatch_thread_id.xy)] = float4(finalcolor, 1.0f);
}
