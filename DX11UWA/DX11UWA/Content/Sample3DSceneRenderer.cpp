#include "pch.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

using namespace DX11UWA;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));

	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

XMFLOAT4 MatrixByVector(XMFLOAT4X4 matrix, XMFLOAT4 vect)
{
	XMFLOAT4 out;
	out.x = matrix._11 * vect.x + matrix._12 * vect.y + matrix._13 * vect.z + matrix._14 * vect.w;
	out.y = matrix._21 * vect.x + matrix._22 * vect.y + matrix._23 * vect.z + matrix._24 * vect.w;
	out.z = matrix._31 * vect.x + matrix._32 * vect.y + matrix._33 * vect.z + matrix._34 * vect.w;
	out.w = matrix._41 * vect.x + matrix._42 * vect.y + matrix._43 * vect.z + matrix._44 * vect.w;
	return out;
}

XMFLOAT4 LookAt(XMFLOAT4 pos, XMFLOAT4 look)
{
	XMFLOAT4 out;
	out.x = look.x - pos.x;
	out.y = look.y - pos.y;
	out.z = look.z - pos.z;
	out.w = look.w - pos.w;
	return out;
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	float Oradians = 1;
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));
		Oradians = radians;
		Orbit(m_constantBufferData,XMFLOAT3(0,0, radians), XMFLOAT3(0, 0, 0), XMFLOAT3(3, 8, 0));
	}
	//set pos of static objects
	Static(scene.models[0].m_constantBufferData, scene.models[0].offset);
	Static(scene.models[1].m_constantBufferData, scene.models[1].offset);
	Static(scene.models[4].m_constantBufferData, scene.models[4].offset);
	Static(scene.models[5].m_constantBufferData, scene.models[5].offset);
	Static(scene.models[6].m_constantBufferData, scene.models[6].offset);
	Static(scene.models[7].m_constantBufferData, scene.models[7].offset);
	Static(scene.models[8].m_constantBufferData, scene.models[8].offset);
	//Orbit(scene.models[6].m_constantBufferData, XMFLOAT3(0,Oradians,0),scene.models[6].offset, XMFLOAT3(0,0,0));
	//make ball and cone obit
	Orbit(scene.models[2].m_constantBufferData, XMFLOAT3(0, Oradians + tan(timer.GetTotalSeconds()), 0), XMFLOAT3(0, 0, 0), scene.models[2].offset);
	Orbit(scene.models[3].m_constantBufferData, XMFLOAT3(0, Oradians, 0), XMFLOAT3(0, 0, 0), scene.models[3].offset);
	//attach lights to ball and cone
	scene.pointlight.pos = MatrixByVector(scene.models[2].m_constantBufferData.model, scene.constPointPos);
	scene.spotlight.pos = MatrixByVector(scene.models[3].m_constantBufferData.model, scene.constspotPos);
	scene.dirlight.dir = LookAt(MatrixByVector(m_constantBufferData.model, XMFLOAT4(3, 8, 0, 0)), XMFLOAT4(0, 0, 0, 0));
	//redirect spotlight
	scene.spotlight.coneDir = LookAt(scene.spotlight.pos, XMFLOAT4(0, 0, 0, 0));

	// Update or move camera here
	UpdateCamera(timer, 7.0f, 1.75f);

}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians) * XMMatrixTranslation(0,3,0)));
}

void Sample3DSceneRenderer::Static(ModelViewProjectionConstantBuffer &objectM, XMFLOAT3 pos)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&objectM.model, XMMatrixTranspose(XMMatrixTranslation(pos.x, pos.y, pos.z)));
}

void Sample3DSceneRenderer::Orbit(ModelViewProjectionConstantBuffer &objectM, XMFLOAT3 radians, XMFLOAT3 orbitpos, XMFLOAT3 orbitness)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&objectM.model, XMMatrixTranspose(XMMatrixTranslation(orbitness.x, orbitness.y, orbitness.z) * (XMMatrixRotationX(radians.x) * XMMatrixRotationY(radians.y) * XMMatrixRotationZ(radians.z)) * XMMatrixTranslation(orbitpos.x,orbitpos.y,orbitpos.z)));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation( 0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_currMousePos) 
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
			float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

			XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

			m_camera._41 = 0;
			m_camera._42 = 0;
			m_camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

			XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
			temp_camera = XMMatrixMultiply(rotX, temp_camera);
			temp_camera = XMMatrixMultiply(temp_camera, rotY);

			XMStoreFloat4x4(&m_camera, temp_camera);

			m_camera._41 = pos.x;
			m_camera._42 = pos.y;
			m_camera._43 = pos.z;
		}
		m_prevMousePos = m_currMousePos;
	}


}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));


	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	// Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

	//////////////////////////////RENDER My Stuff

	context->VSSetShader(scene.m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(scene.m_pixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, scene.m_SamplerState.GetAddressOf());
	context->IASetInputLayout(scene.m_inputLayout.Get());

	context->PSSetConstantBuffers1(0, 1, scene.m_dirConstBuffer.GetAddressOf(), nullptr, nullptr);
	context->UpdateSubresource1(scene.m_dirConstBuffer.Get(), 0, NULL, &scene.dirlight, 0, 0, 0);
	context->PSSetConstantBuffers1(1, 1, scene.m_pointConstBuffer.GetAddressOf(), nullptr, nullptr);
	context->UpdateSubresource1(scene.m_pointConstBuffer.Get(), 0, NULL, &scene.pointlight, 0, 0, 0);
	context->PSSetConstantBuffers1(2, 1, scene.m_spotConstBuffer.GetAddressOf(), nullptr, nullptr);
	context->UpdateSubresource1(scene.m_spotConstBuffer.Get(), 0, NULL, &scene.spotlight, 0, 0, 0);

	for (unsigned int i = 0; i < scene.models.size(); ++i)
	{
		scene.models[i].m_constantBufferData.view = m_constantBufferData.view;
		scene.models[i].m_constantBufferData.projection = m_constantBufferData.projection;
		context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &scene.models[i].m_constantBufferData, 0, 0, 0);

		stride = sizeof(VERTEX);
		offset = 0;
		context->IASetVertexBuffers(0, 1, scene.models[i].m_vertexBuffer.GetAddressOf(), &stride, &offset);

		context->IASetIndexBuffer(scene.models[i].m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
		context->PSSetShaderResources(0, 1, scene.models[i].m_texture.GetAddressOf());
		context->PSSetShaderResources(1, 1, scene.models[i].m_normalMap.GetAddressOf());

		context->DrawIndexed(scene.models[i].indexed.size(), 0, 0);
	}


}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices[] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{
		m_loadingComplete = true;
	});

	//////////////////////////////////////////////////////////////Load My Stuff

	Object plane;
	Object monkey;
	Object ball;
	Object cone;
	Object ball2;
	Object gun;
	Object m4;
	Object cobble;
	Object moon;
	scene.models.push_back(plane);
	scene.models.push_back(monkey);
	scene.models.push_back(ball);
	scene.models.push_back(cone);
	scene.models.push_back(ball2);
	scene.models.push_back(gun);
	scene.models.push_back(m4);
	scene.models.push_back(cobble);
	scene.models.push_back(moon);

	CD3D11_BUFFER_DESC DirconstantBufferDesc(sizeof(DIRECTOIONALLIGHT), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&DirconstantBufferDesc, nullptr, &scene.m_dirConstBuffer));
	CD3D11_BUFFER_DESC PointconstantBufferDesc(sizeof(POINTLIGHT), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&PointconstantBufferDesc, nullptr, &scene.m_pointConstBuffer));
	CD3D11_BUFFER_DESC SpotconstantBufferDesc(sizeof(SPOTLIGHT), D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&SpotconstantBufferDesc, nullptr, &scene.m_spotConstBuffer));

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 1.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc, &scene.m_SamplerState));

	scene.dirlight.dir = XMFLOAT4(-0.5, -0.5, 0, 0);
	scene.dirlight.color = XMFLOAT4(0.35, 0.35, 0.35, 0);
	scene.dirlight.ambientlight = XMFLOAT4(0.00, 0.00, 0.00, 0);

	scene.pointlight.pos = scene.constPointPos = XMFLOAT4(0, 0, -3, 0);
	scene.pointlight.color = XMFLOAT4(0.0, 0.0, 1.0, 0);
	scene.pointlight.radious = XMFLOAT4(3.7, 3, 0, 0);

	scene.spotlight.pos = scene.constspotPos = XMFLOAT4(0, 0, 3, 0);
	scene.spotlight.color = XMFLOAT4(0.56, 0.56, 0.56, 0);
	scene.spotlight.coneRat = XMFLOAT4(0.7708, 0.7956, 10, 0);
	scene.spotlight.coneDir = XMFLOAT4(0, 0, -1, 0);

	// Load shaders asynchronously.
	auto SceneloadVSTask = DX::ReadDataAsync(L"ModelVertexShader.cso");
	auto SceneloadPSTask = DX::ReadDataAsync(L"ModelPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto ScenecreateVSTask = SceneloadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &scene.m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC PlanevertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TAN", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BI", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UNM", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(PlanevertexDesc, ARRAYSIZE(PlanevertexDesc), &fileData[0], fileData.size(), &scene.m_inputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto ScenecreatePSTask = SceneloadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &scene.m_pixelShader));
	});

	// Once both shaders are loaded, create the mesh.
	auto PlanecreateCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[0].loadOBJ("Assets/Plane.obj"))
		{
			XMFLOAT3 tan;
			XMFLOAT3 bi;

			scene.models[0].CalculateTangentBinormal(scene.models[0].verts[scene.models[0].indexed[0]], scene.models[0].verts[scene.models[0].indexed[1]], scene.models[0].verts[scene.models[0].indexed[2]], tan, bi);
			for (int i = 0; i < scene.models[0].verts.size(); ++i)
			{
				scene.models[0].verts[i].tan = tan;
				scene.models[0].verts[i].Bi = bi;
				scene.models[0].verts[i].useNormalMap = 1.0f;
			}
			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[0].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[0].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[0].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[0].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[0].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[0].m_indexBuffer));
			scene.models[0].offset = XMFLOAT3(0, -1.37, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/tiles.dds", NULL, scene.models[0].m_texture.GetAddressOf());
			HRESULT hs2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/tiles_NRM.dds", NULL, scene.models[0].m_normalMap.GetAddressOf());

			
		}
	});

	auto MonkeycreateCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[1].loadOBJ("Assets/Monkey.obj"))
		{

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[1].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[1].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[1].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[1].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[1].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[1].m_indexBuffer));
			scene.models[1].offset = XMFLOAT3(0, 0, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/grey.dds", NULL, &scene.models[1].m_texture);
		}
	});

	auto BallcreateCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[2].loadOBJ("Assets/Ball.obj"))
		{

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[2].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[2].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[2].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[2].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[2].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[2].m_indexBuffer));
			scene.models[2].offset = XMFLOAT3(0, 0, -3);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/blue.dds", NULL, &scene.models[2].m_texture);
		}
	});

	auto ConecreateCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[3].loadOBJ("Assets/Cone.obj"))
		{

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[3].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[3].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[3].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[3].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[3].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[3].m_indexBuffer));
			scene.models[3].offset = XMFLOAT3(0, 0, 3);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/Red.dds", NULL, &scene.models[3].m_texture);
		}
	});

	auto Ball2createCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[4].loadOBJ("Assets/Ball.obj"))
		{

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[4].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[4].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[4].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[4].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[4].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[4].m_indexBuffer));
			scene.models[4].offset = XMFLOAT3(0, 6, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/blue.dds", NULL, &scene.models[4].m_texture);
		}
	});

	auto GuncreateCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[5].loadOBJ("Assets/Handgun_obj.obj"))
		{
			XMFLOAT3 tan;
			XMFLOAT3 bi;

			for (int i = 0; i < scene.models[5].indexed.size(); i += 3)
			{
				scene.models[5].CalculateTangentBinormal(scene.models[5].verts[scene.models[5].indexed[i]], scene.models[5].verts[scene.models[5].indexed[i + 1]], scene.models[5].verts[scene.models[5].indexed[i + 2]], tan, bi);
				scene.models[5].verts[scene.models[5].indexed[i]].tan = tan;
				scene.models[5].verts[scene.models[5].indexed[i]].Bi = bi;
				scene.models[5].verts[scene.models[5].indexed[i]].useNormalMap = 1.0f;
				scene.models[5].verts[scene.models[5].indexed[i + 1]].tan = tan;
				scene.models[5].verts[scene.models[5].indexed[i + 1]].Bi = bi;
				scene.models[5].verts[scene.models[5].indexed[i + 1]].useNormalMap = 1.0f;
				scene.models[5].verts[scene.models[5].indexed[i + 2]].tan = tan;
				scene.models[5].verts[scene.models[5].indexed[i + 2]].Bi = bi;
				scene.models[5].verts[scene.models[5].indexed[i + 2]].useNormalMap = 1.0f;
			}
			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[5].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[5].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[5].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[5].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[5].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[5].m_indexBuffer));
			scene.models[5].offset = XMFLOAT3(-3, 1.37, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/handgun_C.dds", NULL, scene.models[5].m_texture.GetAddressOf());
			HRESULT hs2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/handgun_N.dds", NULL, scene.models[5].m_normalMap.GetAddressOf());


		}
	});

	auto m4createCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[6].loadOBJ("Assets/m4a1_s.obj"))
		{
			/*XMFLOAT3 tan;
			XMFLOAT3 bi;

			for (int i = 0; i < scene.models[6].indexed.size() - 2; i += 3)
			{
				scene.models[6].CalculateTangentBinormal(scene.models[6].verts[scene.models[6].indexed[i]], scene.models[6].verts[scene.models[6].indexed[i + 1]], scene.models[6].verts[scene.models[6].indexed[i + 2]], tan, bi);
				scene.models[6].verts[scene.models[6].indexed[i]].tan = tan;
				scene.models[6].verts[scene.models[6].indexed[i]].Bi = bi;
				scene.models[6].verts[scene.models[6].indexed[i]].useNormalMap = 1.0f;
				scene.models[6].verts[scene.models[6].indexed[i + 1]].tan = tan;
				scene.models[6].verts[scene.models[6].indexed[i + 1]].Bi = bi;
				scene.models[6].verts[scene.models[6].indexed[i + 1]].useNormalMap = 1.0f;
				scene.models[6].verts[scene.models[6].indexed[i + 2]].tan = tan;
				scene.models[6].verts[scene.models[6].indexed[i + 2]].Bi = bi;
				scene.models[6].verts[scene.models[6].indexed[i + 2]].useNormalMap = 1.0f;
			}*/
			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[6].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[6].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[6].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[6].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[6].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[6].m_indexBuffer));
			scene.models[6].offset = XMFLOAT3(-4.6, 1, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/noodas.dds", NULL, scene.models[6].m_texture.GetAddressOf());
			//HRESULT hs2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/craters_NRM.dds", NULL, scene.models[6].m_normalMap.GetAddressOf());


		}
	});

	auto Plane2createCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[7].loadOBJ("Assets/Plane.obj"))
		{
			XMFLOAT3 tan;
			XMFLOAT3 bi;

			scene.models[7].CalculateTangentBinormal(scene.models[7].verts[scene.models[7].indexed[0]], scene.models[7].verts[scene.models[7].indexed[1]], scene.models[7].verts[scene.models[7].indexed[2]], tan, bi);
			for (int i = 0; i < scene.models[7].verts.size(); ++i)
			{
				scene.models[7].verts[i].tan = tan;
				scene.models[7].verts[i].Bi = bi;
				scene.models[7].verts[i].useNormalMap = 1.0f;
			}
			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[7].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[7].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[7].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[7].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[7].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[7].m_indexBuffer));
			scene.models[7].offset = XMFLOAT3(16, -1.37, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/Cobblestone.dds", NULL, scene.models[7].m_texture.GetAddressOf());
			HRESULT hs2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/Cobblestone_NRM.dds", NULL, scene.models[7].m_normalMap.GetAddressOf());


		}
	});

	auto mooncreateCubeTask = (ScenecreatePSTask && ScenecreateVSTask).then([this]()
	{
		if (scene.models[8].loadOBJ("Assets/UVball.obj"))
		{
			XMFLOAT3 tan;
			XMFLOAT3 bi;

			for (int i = 0; i < scene.models[8].indexed.size(); i += 3)
			{
				scene.models[8].CalculateTangentBinormal(scene.models[8].verts[scene.models[8].indexed[i]], scene.models[8].verts[scene.models[8].indexed[i + 1]], scene.models[8].verts[scene.models[8].indexed[i + 2]], tan, bi);
				scene.models[8].verts[scene.models[8].indexed[i]].tan = tan;
				scene.models[8].verts[scene.models[8].indexed[i]].Bi = bi;
				scene.models[8].verts[scene.models[8].indexed[i]].useNormalMap = 1.0f;
				scene.models[8].verts[scene.models[8].indexed[i + 1]].tan = tan;
				scene.models[8].verts[scene.models[8].indexed[i + 1]].Bi = bi;
				scene.models[8].verts[scene.models[8].indexed[i + 1]].useNormalMap = 1.0f;
				scene.models[8].verts[scene.models[8].indexed[i + 2]].tan = tan;
				scene.models[8].verts[scene.models[8].indexed[i + 2]].Bi = bi;
				scene.models[8].verts[scene.models[8].indexed[i + 2]].useNormalMap = 1.0f;
			}
			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = scene.models[8].verts.data();
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VERTEX) * scene.models[8].verts.size(), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &scene.models[8].m_vertexBuffer));

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = scene.models[8].indexed.data();
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * scene.models[8].indexed.size(), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &scene.models[8].m_indexBuffer));
			scene.models[8].offset = XMFLOAT3(0, 4.3, 0);

			HRESULT hs = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/craters.dds", NULL, scene.models[8].m_texture.GetAddressOf());
			HRESULT hs2 = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets/craters_NRM.dds", NULL, scene.models[8].m_normalMap.GetAddressOf());


		}
	});

}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
}