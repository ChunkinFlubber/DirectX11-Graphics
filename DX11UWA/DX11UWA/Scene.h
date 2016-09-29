#pragma once
#include "OBJLoader.h"


class Scene
{
public:
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	vector<Object> models; 
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	bool Render();
	bool SetUp();
};