#pragma once
#include "pch.h"
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Content\ShaderStructures.h"
#include "..\Common\DeviceResources.h"

using namespace std;
using namespace DirectX;

class Object
{
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
	DX11UWA::ModelViewProjectionConstantBuffer	m_constantBufferData;
	XMFLOAT3 offset;

	vector<DX11UWA::VERTEX> verts;
	vector<unsigned int> indexed;

	//Loads OBJ files into vectors of verts, uvs and normals by parsing the file, returns true if success
	bool loadOBJ(const char * path);
	bool SetDX();
};

