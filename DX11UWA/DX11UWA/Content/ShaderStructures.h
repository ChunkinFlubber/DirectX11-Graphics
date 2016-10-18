#pragma once

namespace DX11UWA
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4 eyepos;
	};

	struct ModelViewProjectionConstantBuffer2
	{
		DirectX::XMFLOAT4X4 model[3];
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4 eyepos;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct VertexPositionUVNormal
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 uv;
		DirectX::XMFLOAT3 normal;
	};

	struct VERTEX
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uvs;
		DirectX::XMFLOAT3 normals;
		DirectX::XMFLOAT3 tan;
		DirectX::XMFLOAT3 Bi;
		float useNormalMap;
	};

	struct DIRECTOIONALLIGHT
	{
		DirectX::XMFLOAT4 dir;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 ambientlight;
	};

	struct POINTLIGHT
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 radious;
	};

	struct SPOTLIGHT
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 coneRat;
		DirectX::XMFLOAT4 coneDir;
	};
}