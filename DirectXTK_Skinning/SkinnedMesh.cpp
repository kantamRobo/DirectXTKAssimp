#include "pch.h"
#include "SkinnedMesh.h"


//実装部分

//頂点バッファ・インデックスバッファ・定数バッファの作成

HRESULT SkinnedMesh::CreateBuffers(const DX::DeviceResources* deviceResources, int width, int height)
{
	auto device = deviceResources->GetD3DDevice();
	m_boneMatrixBuffer.Create(deviceResources->GetD3DDevice());

	// 2. 頂点データ例
	const VS_INPUT vertices[] = {
		{ DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.5f, 0.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f), DirectX::XMUINT4(0,0,0,0), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f) },
	};
 vertexCount = std::size(vertices);
	 vertexStride = sizeof(VS_INPUT);
	// 3. D3D11_BUFFER_DESC の設定
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = vertexStride * vertexCount;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = 0;

	// 4. 初期データの設定
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	// 5. バッファ生成
	HRESULT hr = device->CreateBuffer(&vbDesc, &initData, m_vertexBuffer.GetAddressOf());
	if (FAILED(hr)) {
		// エラーハンドリング
		return;
	}
	return S_OK;
}


//パイプラインステートの作成
HRESULT SkinnedMesh::CreateShaders(const DX::DeviceResources* deviceResources)
{
	return S_OK;
}

//描画


void SkinnedMesh::Draw(const DX::DeviceResources* deviceResources)
{
	auto device = deviceResources->GetD3DDevice();
	auto deviceContext = deviceResources->GetD3DDeviceContext();
	auto boneaddress = m_boneMatrixBuffer.GetBuffer();
	//ボーン行列変換用の定数バッファのパイプラインへのセット
	deviceContext->VSSetConstantBuffers(0, 1, &boneaddress);


	// 6. パイプラインへのバインド（例）
	UINT stride = vertexStride;
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
}