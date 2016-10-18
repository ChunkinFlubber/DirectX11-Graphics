// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model[3];
	matrix view;
	matrix projection;
	float4 eyepos;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : UV;
	float3 normals : NORMAL;
	float3 tan : TAN;
	float3 bi : BI;
	float useNormalMap : UNM;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normals : NORMAL;
	float3 normalsw : WORLDNORM;
	float3 posw : WORLDPOS;
	float3 tan : TAN;
	float3 bi : BI;
	float3 eyepos : EYE;
	float useNormalMap : UNM;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input, unsigned int Instance : SV_InstanceID)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model[Instance]);
	output.posw = pos.xyz;
	output.normalsw = normalize(mul(input.normals, model[Instance]));
	output.eyepos = mul(eyepos, model[Instance]);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = float3(input.uv, 1.0f);
	output.normals = input.normals;

	if (input.useNormalMap == 1)
	{
		output.tan = mul(input.tan, model[Instance]);
		output.bi = mul(input.bi, model[Instance]);
	}
	output.useNormalMap = input.useNormalMap;

	return output;
}