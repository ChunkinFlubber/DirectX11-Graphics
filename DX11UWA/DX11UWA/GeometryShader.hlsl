
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 eyepos;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
};

struct GeoIn
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
};

[maxvertexcount(4)]
void main(point GeoIn input[1], inout TriangleStream< PixelShaderInput > output)
{
	PixelShaderInput temp[4] = { (PixelShaderInput)0,(PixelShaderInput)0,(PixelShaderInput)0,(PixelShaderInput)0 };

	for (uint i = 0; i < 4; i++)
	{
		temp[i].pos = input[0].pos;
	}

	temp[0].pos.x -= 3;
	temp[0].pos.y += 3;
	temp[0].uv.x = 0;
	temp[0].uv.y = 0;
	temp[0].pos = mul(temp[0].pos, model);
	temp[0].pos = mul(temp[0].pos, view);
	temp[0].pos = mul(temp[0].pos, projection);
	output.Append(temp[0]);
	temp[1].pos.x += 3;
	temp[1].pos.y += 3;
	temp[1].uv.x = 1;
	temp[1].uv.y = 0;
	temp[1].pos = mul(temp[1].pos, model);
	temp[1].pos = mul(temp[1].pos, view);
	temp[1].pos = mul(temp[1].pos, projection);
	output.Append(temp[1]);
	temp[2].pos.x -= 3;
	temp[2].pos.y -= 3;
	temp[2].uv.x = 0;
	temp[2].uv.y = 1;
	temp[2].pos = mul(temp[2].pos, model);
	temp[2].pos = mul(temp[2].pos, view);
	temp[2].pos = mul(temp[2].pos, projection);
	output.Append(temp[2]);
	temp[3].pos.x += 3;
	temp[3].pos.y -= 3;
	temp[3].uv.x = 1;
	temp[3].uv.y = 1;
	temp[3].pos = mul(temp[3].pos, model);
	temp[3].pos = mul(temp[3].pos, view);
	temp[3].pos = mul(temp[3].pos, projection);
	output.Append(temp[3]);
}