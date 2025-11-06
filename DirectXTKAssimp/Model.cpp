#include "pch.h"




#define NOMINMAX
#include <BufferHelpers.h>
#include <iostream>
#include <cassert>
#include <d3dcompiler.h>
#include <algorithm>
#include <BufferHelpers.h>
#include <functional>
#include "Model.h"
#include <map>


#pragma comment(lib, "d3dcompiler.lib")

using namespace education;
//TODO:コンストラクタでモデルロード・バッファ作成・シェーダー作成を行う



education::Model::Model(DX::DeviceResources* deviceresources,const char* path,int width,int height)
{
    if (!LoadModel(path))
    {
		std::abort();
    }
    
/*
追加分
*/
    if (FAILED(CreateBuffers(deviceresources, width,height)))
    {
		std::abort();
    }
	if(FAILED(CreateShaders(deviceresources)))
	{
		std::abort();
	}
	if(FAILED(craetepipelineState(deviceresources)))
	{
		std::abort();
	}

}

bool education::Model::LoadModel(const char* path)
{


    m_scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals);
    if (!m_scene || !m_scene->mRootNode)
    {
		auto error = m_importer.GetErrorString();
		OutputDebugStringA(error);
        return false;
    }
    vertices = GenerateVertices();
    // スケール値を設定
    float scaleFactor = 10.0f;


    for (int i = 0; i < vertices.size(); i++)
    {
        //乗算　vertices[i].position = vertices[i].position,100.0f;
    }

    // 頂点数が0でないことを確認
    assert(vertices.size() > 0 && "頂点の生成に失敗");

    // インデックス数が3の倍数であり、かつインデックスが頂点数を超えないことを確認
    assert(indices.size() % 3 == 0 && "インデックス数が3の倍数でない");
    assert(indices.size() <= vertices.size() * 3 && "インデックスが頂点数を超えています");

    return true;
}

std::vector<DirectX::VertexPositionNormalTangentColorTextureSkinning> education::Model::GenerateVertices()
{
    assert(m_scene);



    std::vector<DirectX::VertexPositionNormalTangentColorTextureSkinning> outvertices;
    outvertices.clear();

    for (unsigned int i = 0; i < m_scene->mNumMeshes; i++)
    {
        aiMesh* mesh = m_scene->mMeshes[i];



        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            DirectX::VertexPositionNormalTangentColorTextureSkinning vertex = {};
            aiVector3D pos = mesh->mVertices[j];


            vertex.position = { pos.x , pos.y , pos.z };
            vertex.normal = { mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z };

            if (mesh->mTextureCoords[0])
            {
                vertex.textureCoordinate.x = mesh->mTextureCoords[0][j].x;
                vertex.textureCoordinate.y = mesh->mTextureCoords[0][j].y;
            }
            else
            {
                vertex.textureCoordinate.x = 0.0f;
                vertex.textureCoordinate.y = 0.0f;
            }

            outvertices.push_back(vertex);
        }

        // インデックスの設定
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }
    }

    

    return outvertices;
}



HRESULT education::Model::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //パイプラインステートの作成
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // 頂点シェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
auto hr =D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
		return hr;
	}

	// 頂点シェーダーの作成
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
	if (FAILED(hr))
	{
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
		return hr;
	}


    // ピクセルシェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    //ピクセルシェーダーの作成
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
	if (FAILED(hr))
	{
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
		return hr;
	}
	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
       // { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
       // { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_modelInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}

	


    return hr;
}

HRESULT education::Model::CreateBuffers(const DX::DeviceResources* deviceResources, int width, int height)
{
    auto device = deviceResources->GetD3DDevice();
    auto context = deviceResources->GetD3DDeviceContext();
    // Vertex Buffer Description
    auto vertexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(DirectX::VertexPositionNormalColorTexture) * vertices.size(), // Total size
        D3D11_BIND_VERTEX_BUFFER,                                           // Bind as vertex buffer
        D3D11_USAGE_DYNAMIC,                                                // Dynamic usage
        D3D11_CPU_ACCESS_WRITE                                              // Allow CPU write access
    );

    // Create Vertex Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBufferTemp;
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(device,
            vertices.data(),                // const void *ptr
            vertices.size(),     // size_t count
            sizeof(VertexPositionNormalColorTexture), // size_t stride
            D3D11_BIND_VERTEX_BUFFER, &vertexBufferTemp)
    );


    // Store as ID3D11Resource
    m_vertexBuffer = vertexBufferTemp;

    // Index Buffer Description
    auto indexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(UINT) * indices.size(), // Total size
        D3D11_BIND_INDEX_BUFFER,       // Bind as index buffer
        D3D11_USAGE_DYNAMIC,           // Dynamic usage
        D3D11_CPU_ACCESS_WRITE         // Allow CPU write access
    );
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBufferTemp;
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(device,
            indices.data(),                // const void *ptr
            indices.size(),     // size_t count
            sizeof(unsigned short), // size_t stride
            D3D11_BIND_INDEX_BUFFER, &indexBufferTemp)
    );


    // Store as ID3D11Resource
    m_indexBuffer = indexBufferTemp;

    // Create Constant Buffer
    m_constantBuffer.Create(device);

	//m_boneBuffer.Create(device);

   
    // Set up Matrices
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -10.0f, 0.0f);
    DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

    float fov = DirectX::XMConvertToRadians(45.0f);
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float nearZ = 0.1f;
    float farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    // Update Constant Buffer
    SceneCB cb = {};
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

    m_constantBuffer.SetData(deviceResources->GetD3DDeviceContext(), cb);
  
    return S_OK;
}



HRESULT education::Model::craetepipelineState(const DX::DeviceResources* deviceResources)
{

    D3D11_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;


	auto hr =deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());



	if (FAILED(hr))
	{
		return hr;
	}

    return hr;
}





void education::Model::Draw(const DX::DeviceResources* DR) {
    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("Vertex or index buffer is empty.\n");
        return;
    }

    auto context = DR->GetD3DDeviceContext();

    // Input Layout 設定
    context->IASetInputLayout(m_modelInputLayout.Get());
   
    // インデックスバッファの設定
    auto indexBuffer = static_cast<ID3D11Buffer*>(m_indexBuffer.Get());
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // 頂点バッファの設定
    UINT stride = sizeof(DirectX::VertexPositionNormalColorTexture);
    UINT offset = 0;
    auto vertexBuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    // プリミティブトポロジー設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto buffer = m_constantBuffer.GetBuffer();
    context->VSSetConstantBuffers(0, 1, &buffer);
    context->PSSetConstantBuffers(0, 1, &buffer);

    // シェーダー設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    context->PSSetShaderResources(0, 1, m_modelsrv.GetAddressOf());
    // 描画コール   
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}





