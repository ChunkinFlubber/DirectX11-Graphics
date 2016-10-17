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

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_RTT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DSTX;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_RTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_DSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_SRV1;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>  Sky_Sampler;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	Sky_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	Cam_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	Sky_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		Sky_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		Sky_indexBuffer;
	DX11UWA::ModelViewProjectionConstantBuffer	Sky_constantBufferData;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	Sky_texture = nullptr;

	DX11UWA::DIRECTOIONALLIGHT dirlight;
	DX11UWA::POINTLIGHT pointlight;
	DX11UWA::SPOTLIGHT spotlight;
	XMFLOAT4 constPointPos;
	XMFLOAT4 constspotPos;
	bool Render();
	bool SetUp();
};