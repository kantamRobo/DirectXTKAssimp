#include "pch.h"
#include "SkinnedMesh.h"

// 頂点バッファ・インデックスバッファ・定数バッファの作成
HRESULT SkinnedMesh::CreateBuffers(const DX::DeviceResources* deviceResources, int width, int height)
{
	auto device = deviceResources->GetD3DDevice();
	m_boneMatrixBuffer.Create(deviceResources->GetD3DDevice());

	// 頂点データ例
	const VS_INPUT vertices[] = {
		{ DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.5f, 0.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
	};
	vertexCount = std::size(vertices);
	vertexStride = sizeof(VS_INPUT);

	// D3D11_BUFFER_DESC の設定
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = vertexStride * vertexCount;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	// 初期データの設定
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	// 頂点バッファ生成
	HRESULT hr = device->CreateBuffer(&vbDesc, &initData, m_vertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	// インデックスバッファの作成
	const UINT indices[] = { 0, 1, 2 };
	UINT indexCount = std::size(indices);

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(UINT) * indexCount;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitData = {};
	indexInitData.pSysMem = indices;
	indexInitData.SysMemPitch = 0;
	indexInitData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&ibDesc, &indexInitData, m_indexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	// シーン用定数バッファの作成
	sceneConstantBuffer.Create(device);

	return S_OK;
}

// シェーダーの作成
HRESULT SkinnedMesh::CreateShaders(const DX::DeviceResources* deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	HRESULT hr = S_OK;

	// シェーダーファイルのコンパイル（ファイルから読み込む場合の例）
	// 実装時に適切なシェーダーファイルパスを指定してください

	// 頂点シェーダーのコンパイル例
	hr = D3DCompileFromFile(
		L"SkinnedMesh.hlsl",
		nullptr,
		nullptr,
		"VS_Main",
		"vs_5_0",
		0,
		0,
		m_pVSBlob.GetAddressOf(),
		nullptr
	);
	if (FAILED(hr)) {
		return hr;
	}

	// 頂点シェーダーの作成
	hr = device->CreateVertexShader(
		m_pVSBlob->GetBufferPointer(),
		m_pVSBlob->GetBufferSize(),
		nullptr,
		m_vertexShader.GetAddressOf()
	);
	if (FAILED(hr)) {
		return hr;
	}

	// ピクセルシェーダーのコンパイル例
	hr = D3DCompileFromFile(
		L"SkinnedMesh.hlsl",
		nullptr,
		nullptr,
		"PS_Main",
		"ps_5_0",
		0,
		0,
		m_pPSBlob.GetAddressOf(),
		nullptr
	);
	if (FAILED(hr)) {
		return hr;
	}

	// ピクセルシェーダーの作成
	hr = device->CreatePixelShader(
		m_pPSBlob->GetBufferPointer(),
		m_pPSBlob->GetBufferSize(),
		nullptr,
		m_pixelShader.GetAddressOf()
	);
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

// パイプラインステートの作成
HRESULT SkinnedMesh::craetepipelineState(const DX::DeviceResources* deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	HRESULT hr = S_OK;

	// 入力レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// 入力レイアウトの作成
	if (m_pVSBlob)
	{
		hr = device->CreateInputLayout(
			layout,
			numElements,
			m_pVSBlob->GetBufferPointer(),
			m_pVSBlob->GetBufferSize(),
			m_inputLayout.GetAddressOf()
		);
		if (FAILED(hr)) {
			return hr;
		}
	}

	// ラスタライザーステートの作成
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;

	hr = device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	// CommonStates の初期化
	m_commonStates = std::make_unique<DirectX::CommonStates>(device);

	return S_OK;
}

// 描画
void SkinnedMesh::Draw(const DX::DeviceResources* deviceResources)
{
	auto deviceContext = deviceResources->GetD3DDeviceContext();

	// パイプラインへのバインド
	UINT stride = vertexStride;
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// シェーダーのセット
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// ラスタライザーステートのセット
	deviceContext->RSSetState(m_rasterizerState.Get());

	// ボーン行列変換用の定数バッファのセット
	auto boneaddress = m_boneMatrixBuffer.GetBuffer();
	deviceContext->VSSetConstantBuffers(0, 1, &boneaddress);

	// シーン定数バッファのセット
	auto sceneAddress = sceneConstantBuffer.GetBuffer();
	deviceContext->VSSetConstantBuffers(1, 1, &sceneAddress);

	// インデックスバッファでの描画
	deviceContext->DrawIndexed(3, 0, 0); // 3個のインデックス、オフセット0
}
