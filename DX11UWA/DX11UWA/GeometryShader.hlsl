struct GSOutput
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
void main(point GeoIn input[1], inout TriangleStream< GSOutput > output)
{
	GSOutput temp[4] = { (GSOutput)0,(GSOutput)0,(GSOutput)0,(GSOutput)0 };

	for (uint i = 0; i < 4; i++)
	{
		temp[i].pos = input[0].pos;
	}

	temp[0].pos.x -= 3;
	temp[0].pos.y += 3;
	temp[0].uv.x = 0;
	temp[0].uv.y = 0;
	output.Append(temp[0]);
	temp[1].pos.x += 3;
	temp[1].pos.y += 3;
	temp[1].uv.x = 1;
	temp[1].uv.y = 0;
	output.Append(temp[1]);
	temp[2].pos.x -= 3;
	temp[2].pos.y -= 3;
	temp[2].uv.x = 0;
	temp[2].uv.y = 1;
	output.Append(temp[2]);
	temp[3].pos.x += 3;
	temp[3].pos.y += 3;
	temp[3].uv.x = 1;
	temp[3].uv.y = 1;
	output.Append(temp[3]);
}