#include "pch.h"
#include "SkinnedMesh.h"
#include <d3dcompiler.h>
#include <algorithm>

// 頂点バッファ・インデックスバッファ・定数バッファの作成
HRESULT SkinnedMesh::CreateBuffers(const DX::DeviceResources* deviceResources, int width, int height)
{
	auto device = deviceResources->GetD3DDevice();
	// ボーンバッファをデバイス上に作成（64行列分）
m_boneMatrixBuffer.Create(device);
	

	// シーンバッファを作成（W,V,P）
	 sceneConstantBuffer.Create(device);
	

	// 頂点データ例
	const VS_INPUT vertices[] = {
		{ DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.5f, 0.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) }, // 頂点0 -> ボーン0
		{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMUINT4(1,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) }, // 頂点1 -> ボーン1
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMUINT4(2,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) }, // 頂点2 -> ボーン2
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
auto	hr = device->CreateBuffer(&vbDesc, &initData, m_vertexBuffer.GetAddressOf());
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

	return S_OK;
}

void SkinnedMesh::CreatePipelineStates(const DX::DeviceResources* deviceResources, SkinnedMesh* mesh)
{
	auto device = deviceResources->GetD3DDevice();
	m_commonStates = std::make_unique<DirectX::CommonStates>(deviceResources->GetD3DDevice());
	// SkinnedMesh::CreateShaders の末尾付近（m_pVSBlob がある状態）
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,   0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,  24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_UINT,0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	auto hr = device->CreateInputLayout(layout, _countof(layout),
		m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize(),
		m_inputLayout.GetAddressOf());
	
}

// ボーン行列データを更新して GPU に転送する
HRESULT SkinnedMesh::UpdateBoneTransforms(const DX::DeviceResources* deviceResources, const std::vector<DirectX::XMFLOAT4X4>& boneTransforms)
{
	SkinnedMesh::BoneMatricesCB cb = {};
	// 最小サイズをコピー（最大64）
	size_t copyCount = std::min<size_t>(boneTransforms.size(), 64);
	for (size_t i = 0; i < copyCount; ++i)
	{
		// XMFLOAT4X4 は 16 floats
		const float* src = reinterpret_cast<const float*>(&boneTransforms[i]);
		std::copy(src, src + 16, std::begin(cb.BoneTransforms[i]));
	}
	// 残りは単位行列で埋める（必要なら）
	for (size_t i = copyCount; i < 64; ++i)
	{
		// identity
		float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
		std::copy(std::begin(identity), std::end(identity), std::begin(cb.BoneTransforms[i]));
	}

	auto ctx = deviceResources->GetD3DDeviceContext();
	m_boneMatrixBuffer.SetData(ctx, cb);
	return S_OK;
}

// シーン行列 (World, View, Projection) を更新して GPU に転送する
HRESULT SkinnedMesh::UpdateSceneMatrices(const DX::DeviceResources* deviceResources, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj)
{
	SkinnedMesh::SceneCB cb = {};
	// 直接コピー
	const float* w = reinterpret_cast<const float*>(&world);
	const float* v = reinterpret_cast<const float*>(&view);
	const float* p = reinterpret_cast<const float*>(&proj);
	std::copy(w, w + 16, std::begin(cb.World));
	std::copy(v, v + 16, std::begin(cb.View));
	std::copy(p, p + 16, std::begin(cb.Projection));

	auto ctx = deviceResources->GetD3DDeviceContext();
	sceneConstantBuffer.SetData(ctx, cb);
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

	// ボーン行列変換用の定数バッファのセット（slot 0）
	auto boneaddress = m_boneMatrixBuffer.GetBuffer();
	deviceContext->VSSetConstantBuffers(0, 1, &boneaddress);

	// シーン定数バッファのセット（slot 1）
	auto sceneAddress = sceneConstantBuffer.GetBuffer();
	deviceContext->VSSetConstantBuffers(1, 1, &sceneAddress);

	// インデックスバッファでの描画
	deviceContext->DrawIndexed(3, 0, 0); // 3個のインデックス、オフセット0
}

// シェーダーの作成
HRESULT SkinnedMesh::CreateShaders(const DX::DeviceResources* deviceResources)
{
    auto device = deviceResources->GetD3DDevice();
    HRESULT hr = S_OK;

    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    // 頂点シェーダーのコンパイル
    hr = D3DCompileFromFile(
        L"SkinnedMeshVertex.hlsl",
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        m_pVSBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );
    if (FAILED(hr))
    {
        if (errorBlob) {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return hr;
    }

    // 頂点シェーダー作成
    hr = device->CreateVertexShader(
        m_pVSBlob->GetBufferPointer(),
        m_pVSBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.GetAddressOf()
    );
    if (FAILED(hr)) {
        return hr;
    }

    // ピクセルシェーダーのコンパイル
    errorBlob.Reset();
    hr = D3DCompileFromFile(
        L"SkinnedMeshPixelShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS,
        0,
        m_pPSBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );
    if (FAILED(hr))
    {
        if (errorBlob) {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return hr;
    }

    // ピクセルシェーダー作成
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
