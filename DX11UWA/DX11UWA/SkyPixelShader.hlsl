texture2D map : register(t0);
SamplerState Sampler : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return map.Sample(Sampler, input.uv);
}