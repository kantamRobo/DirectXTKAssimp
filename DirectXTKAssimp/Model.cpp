#include "pch.h"




#define NOMINMAX
#include <BufferHelpers.h>
#include <iostream>
#include <cassert>
#include <d3dcompiler.h>
#include <algorithm>
#include <functional>
#include "Model.h"

#pragma comment(lib, "d3dcompiler.lib")


bool education::Model::LoadModel(const char* path)
{


    m_scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals);
    if (!m_scene || !m_scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << m_importer.GetErrorString() << std::endl;
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

std::vector<DirectX::VertexPositionNormalColorTexture> education::Model::GenerateVertices()
{
    assert(m_scene);



    std::vector< DirectX::VertexPositionNormalColorTexture> outvertices;
    outvertices.clear();

    for (unsigned int i = 0; i < m_scene->mNumMeshes; i++)
    {
        aiMesh* mesh = m_scene->mMeshes[i];



        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            DirectX::VertexPositionNormalColorTexture vertex = {};
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
auto hr =D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		return hr;
	}

	// 頂点シェーダーの作成
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}


    // ピクセルシェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        return hr;
    }

    //ピクセルシェーダーの作成
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
	// 入力レイアウトの作成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_spriteInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}
}

void education::Model::CreateBuffers(const DX::DeviceResources* deviceResources)
{
    //頂点バッファの作成
    DirectX::CreateStaticBuffer(deviceResources->GetD3DDevice(), vertices.data(), vertices.size(), sizeof(DirectX::VertexPositionNormalColorTexture), D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer.GetAddressOf());
    //インデックスバッファの作成
    DirectX::CreateStaticBuffer(deviceResources->GetD3DDevice(), indices.data(), indices.size(), sizeof(UINT), D3D11_BIND_INDEX_BUFFER, m_indexBuffer.GetAddressOf());
}


void education::Model::craetepipelineState(const DX::DeviceResources* deviceResources)
{

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    // 注意
    rasterizerDesc.FrontCounterClockwise = true;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.AntialiasedLineEnable = false;


}





void education::Model::Draw(const DX::DeviceResources* DR) {
    if (vertices.empty() || indices.empty()) {
        return;
    }
    auto device = DR->GetD3DDevice();
    UINT size = sizeof(DirectX::VertexPositionNormalColorTexture);
  auto context = DR->GetD3DDeviceContext();
  context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),&size, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
  context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
  
  context->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}





education::Model::Model(const char* path)
{
    assert(LoadModel(path), "ロードに失敗");
    vertices = GenerateVertices();
}
