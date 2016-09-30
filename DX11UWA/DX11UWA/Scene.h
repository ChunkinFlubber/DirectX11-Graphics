#pragma once
#include "OBJLoader.h"
#include "DDSTextureLoader.h"

class Scene
{
public:
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	vector<Object> models; 
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>  m_SamplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_dirConstBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_pointConstBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_spotConstBuffer;
	DX11UWA::DIRECTOIONALLIGHT dirlight;
	DX11UWA::POINTLIGHT pointlight;
	DX11UWA::SPOTLIGHT spotlight;
	XMFLOAT4 constPointPos;
	XMFLOAT4 constspotPos;
	bool Render();
	bool SetUp();
};