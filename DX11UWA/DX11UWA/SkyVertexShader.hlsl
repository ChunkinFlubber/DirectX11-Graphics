cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 eyepos;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 uv : UV;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
};


PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos + eyepos, 1);
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
	output.uv = input.uv;
	return output;
}