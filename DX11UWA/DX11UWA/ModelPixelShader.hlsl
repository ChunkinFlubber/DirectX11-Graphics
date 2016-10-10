//// Per-pixel color data passed through the pixel shader.

texture2D Texture : register(t0); // first texture

texture2D BumpMap : register(t1); // first texture

texture2D SpecMap : register(t2); // first texture

SamplerState Sampler : register(s0); // filter 0 using CLAMP, filter 1 using WRAP

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

cbuffer DirectionLight : register(b0)
{
	float4 dir_dir;
	float4 dir_color;
	float4 ambiantlight;
};

cbuffer PointLight : register(b1)
{
	float4 point_pos;
	float4 point_color;
	float4 point_radious;
};

cbuffer SpotLight : register(b2)
{
	float4 spot_pos;
	float4 spot_color;
	float4 spot_coneR;
	float4 spot_coneD;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 baseColor = Texture.Sample(Sampler, input.uv);// * modulate; // get base color
	float4 bumpMap;
	float3 bumpNormal = input.normalsw;
	if (input.useNormalMap == 1)
	{
		bumpMap = BumpMap.Sample(Sampler, input.uv);
		bumpMap = (bumpMap * 2.0f) - 1.0f;
		bumpNormal = (bumpMap.x * input.tan) + (bumpMap.y * input.bi) + (bumpMap.z * input.normalsw);
		bumpNormal = normalize(bumpNormal);
	}

	float3 tocam = normalize(input.eyepos - input.posw);

	float3 todirlight = normalize(-dir_dir - input.posw);
	float3 dirRef = reflect(-tocam, input.normalsw);
	float ddirRef = max(0,dot(dirRef, todirlight));
	float dirspecPow = pow(ddirRef, 20);

	float3 topointlight = normalize(point_pos - input.posw);
	float3 pointRef = reflect(-tocam, input.normalsw);
	float dpointRef = max(0, dot(pointRef, topointlight));
	float pointspecPow = pow(dpointRef, 20);

	float3 tospotlight = normalize(spot_pos - input.posw);
	float3 spotRef = reflect(-tocam, input.normalsw);
	float dspotRef = max(0, dot(spotRef, tospotlight));
	float spotspecPow = pow(dspotRef, 20);

	float d_r = saturate(dot(-dir_dir.xyz,bumpNormal));
	float3 dirlight = baseColor.xyz * dir_color.xyz * d_r;
	float3 dirlightSpec = baseColor * dir_color * dirspecPow;

	float3 p_d = normalize(point_pos.xyz - input.posw);
	float p_r = saturate(dot(p_d, bumpNormal));
	float p_atten = 1 - saturate(length(point_pos.xyz - input.posw.xyz) / point_radious.x);
	float3 pointlight = point_color.xyz * baseColor.xyz * p_r * p_atten;
	float3 pointSpec = pointspecPow * point_color * baseColor;

	float3 s_d = normalize(spot_pos.xyz - input.posw);
	float s_sr = saturate(dot(-s_d.xyz, normalize(spot_coneD).xyz));
	float s_sf = (s_sr > spot_coneR.x) ? 1 : 0;
	float s_r = saturate(dot(s_d, bumpNormal));
	float s_atten = 1.0 - saturate(length(spot_pos.xyz - input.posw.xyz) / spot_coneR.z);
	float s_coneatten = 1.0 - saturate((spot_coneR.y - s_sr) / (spot_coneR.y - spot_coneR.x));
	float3 spotlight = spot_color.xyz * baseColor.xyz * s_r * s_sf * (s_atten * s_coneatten);
	float3 spotlightspec = baseColor * spot_color * spotspecPow;

	

	baseColor.xyz = saturate(dirlight.xyz + pointlight.xyz + ambiantlight.xyz + spotlight.xyz + dirlightSpec + pointSpec + spotlightspec);
	//float4 detailColor = detailTexture.Sample(filters[1], detailUV); // get detail effect
	//float4 finalColor = float4(lerp(baseColor.rgb, detailColor.rgb, detailColor.a), baseColor.a);
	return baseColor; // return a transition based on the detail alpha
}

//#define MAX_LIGHTS 8
//
//// Light types.
//#define DIRECTIONAL_LIGHT 0
//#define POINT_LIGHT 1
//#define SPOT_LIGHT 2
//
//Texture2D Texture : register(t0);
//sampler Sampler : register(s0);
//
//struct _Material
//{
//	float4  Emissive;       // 16 bytes
//							//----------------------------------- (16 byte boundary)
//	float4  Ambient;        // 16 bytes
//							//------------------------------------(16 byte boundary)
//	float4  Diffuse;        // 16 bytes
//							//----------------------------------- (16 byte boundary)
//	float4  Specular;       // 16 bytes
//							//----------------------------------- (16 byte boundary)
//	float   SpecularPower;  // 4 bytes
//	bool    UseTexture;     // 4 bytes
//	float2  Padding;        // 8 bytes
//							//----------------------------------- (16 byte boundary)
//};  // Total:               // 80 bytes ( 5 * 16 )
//
//cbuffer MaterialProperties : register(b0)
//{
//	_Material Material;
//};
//
//struct Light
//{
//	float4      Position;               // 16 bytes
//										//----------------------------------- (16 byte boundary)
//	float4      Direction;              // 16 bytes
//										//----------------------------------- (16 byte boundary)
//	float4      Color;                  // 16 bytes
//										//----------------------------------- (16 byte boundary)
//	float       SpotAngle;              // 4 bytes
//	float       ConstantAttenuation;    // 4 bytes
//	float       LinearAttenuation;      // 4 bytes
//	float       QuadraticAttenuation;   // 4 bytes
//										//----------------------------------- (16 byte boundary)
//	int         LightType;              // 4 bytes
//	bool        Enabled;                // 4 bytes
//	int2        Padding;                // 8 bytes
//										//----------------------------------- (16 byte boundary)
//};  // Total:                           // 80 bytes (5 * 16)
//
//cbuffer LightProperties : register(b1)
//{
//	float4 EyePosition;                 // 16 bytes
//										//----------------------------------- (16 byte boundary)
//	float4 GlobalAmbient;               // 16 bytes
//										//----------------------------------- (16 byte boundary)
//	Light Lights[MAX_LIGHTS];           // 80 * 8 = 640 bytes
//};  // Total:                           // 672 bytes (42 * 16)
//
//float4 DoDiffuse(Light light, float3 L, float3 N)
//{
//	float NdotL = max(0, dot(N, L));
//	return light.Color * NdotL;
//}
//
//float4 DoSpecular(Light light, float3 V, float3 L, float3 N)
//{
//	// Phong lighting.
//	float3 R = normalize(reflect(-L, N));
//	float RdotV = max(0, dot(R, V));
//
//	// Blinn-Phong lighting
//	float3 H = normalize(L + V);
//	float NdotH = max(0, dot(N, H));
//
//	return light.Color * pow(RdotV, Material.SpecularPower);
//}
//
//float DoAttenuation(Light light, float d)
//{
//	return 1.0f / (light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d);
//}
//
//struct LightingResult
//{
//	float4 Diffuse;
//	float4 Specular;
//};
//
//LightingResult DoPointLight(Light light, float3 V, float4 P, float3 N)
//{
//	LightingResult result;
//
//	float3 L = (light.Position - P).xyz;
//	float distance = length(L);
//	L = L / distance;
//
//	float attenuation = DoAttenuation(light, distance);
//
//	result.Diffuse = DoDiffuse(light, L, N) * attenuation;
//	result.Specular = DoSpecular(light, V, L, N) * attenuation;
//
//	return result;
//}
//
//LightingResult DoDirectionalLight(Light light, float3 V, float4 P, float3 N)
//{
//	LightingResult result;
//
//	float3 L = -light.Direction.xyz;
//
//	result.Diffuse = DoDiffuse(light, L, N);
//	result.Specular = DoSpecular(light, V, L, N);
//
//	return result;
//}
//
//float DoSpotCone(Light light, float3 L)
//{
//	float minCos = cos(light.SpotAngle);
//	float maxCos = (minCos + 1.0f) / 2.0f;
//	float cosAngle = dot(light.Direction.xyz, -L);
//	return smoothstep(minCos, maxCos, cosAngle);
//}
//
//LightingResult DoSpotLight(Light light, float3 V, float4 P, float3 N)
//{
//	LightingResult result;
//
//	float3 L = (light.Position - P).xyz;
//	float distance = length(L);
//	L = L / distance;
//
//	float attenuation = DoAttenuation(light, distance);
//	float spotIntensity = DoSpotCone(light, L);
//
//	result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity;
//	result.Specular = DoSpecular(light, V, L, N) * attenuation * spotIntensity;
//
//	return result;
//}
//
//LightingResult ComputeLighting(float4 P, float3 N)
//{
//	float3 V = normalize(EyePosition - P).xyz;
//
//	LightingResult totalResult = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };
//
//	[unroll]
//	for (int i = 0; i < MAX_LIGHTS; ++i)
//	{
//		LightingResult result = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 } };
//
//		if (!Lights[i].Enabled) continue;
//
//		switch (Lights[i].LightType)
//		{
//		case DIRECTIONAL_LIGHT:
//		{
//			result = DoDirectionalLight(Lights[i], V, P, N);
//		}
//		break;
//		case POINT_LIGHT:
//		{
//			result = DoPointLight(Lights[i], V, P, N);
//		}
//		break;
//		case SPOT_LIGHT:
//		{
//			result = DoSpotLight(Lights[i], V, P, N);
//		}
//		break;
//		}
//		totalResult.Diffuse += result.Diffuse;
//		totalResult.Specular += result.Specular;
//	}
//
//	totalResult.Diffuse = saturate(totalResult.Diffuse);
//	totalResult.Specular = saturate(totalResult.Specular);
//
//	return totalResult;
//}
//
//struct PixelShaderInput
//{
//	float4 PositionWS   : TEXCOORD1;
//	float3 NormalWS     : TEXCOORD2;
//	float2 TexCoord     : TEXCOORD0;
//};
//
//float4 TexturedLitPixelShader(PixelShaderInput IN) : SV_TARGET
//{
//	LightingResult lit = ComputeLighting(IN.PositionWS, normalize(IN.NormalWS));
//
//float4 emissive = Material.Emissive;
//float4 ambient = Material.Ambient * GlobalAmbient;
//float4 diffuse = Material.Diffuse * lit.Diffuse;
//float4 specular = Material.Specular * lit.Specular;
//
//float4 texColor = { 1, 1, 1, 1 };
//
//if (Material.UseTexture)
//{
//	texColor = Texture.Sample(Sampler, IN.TexCoord);
//}
//
//float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;
//
//return finalColor;
//}
