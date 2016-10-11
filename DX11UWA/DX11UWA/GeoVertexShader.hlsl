struct VSIn
{
	float4 pos : POSTION;
	float3 uv : UV;
};

struct GeoIn
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
};

GeoIn main( VSIn vert )
{
	GeoIn temp;
	temp.pos = vert.pos;
	temp.uv = vert.uv;
	return temp;
}