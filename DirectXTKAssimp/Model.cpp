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
    if (FAILED(CreateBuffers(deviceresources,width,height)))
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


void education::Model::GenerateBones()
{
   
    // ボーン情報を取得
    for (unsigned int meshIndex = 0; meshIndex < m_scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = m_scene->mMeshes[meshIndex];
        if (mesh->HasBones()) {
            for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                aiBone* l_aibone = mesh->mBones[boneIndex];
                std::string boneName = l_aibone->mName.C_Str();

               
                m_bone.offset._11 = l_aibone->mOffsetMatrix.a1;
				m_bone.offset._12 = l_aibone->mOffsetMatrix.a2;
				m_bone.offset._13 = l_aibone->mOffsetMatrix.a3;
				m_bone.offset._14 = l_aibone->mOffsetMatrix.a4;
                
				m_bone.offset._21 = l_aibone->mOffsetMatrix.b1;
				m_bone.offset._22 = l_aibone->mOffsetMatrix.b2;
				m_bone.offset._23 = l_aibone->mOffsetMatrix.b3;
				m_bone.offset._24 = l_aibone->mOffsetMatrix.b4;

				m_bone.offset._31 = l_aibone->mOffsetMatrix.c1;
				m_bone.offset._32 = l_aibone->mOffsetMatrix.c2;
				m_bone.offset._33 = l_aibone->mOffsetMatrix.c3;
				m_bone.offset._34 = l_aibone->mOffsetMatrix.c4;

				m_bone.offset._41 = l_aibone->mOffsetMatrix.d1;
				m_bone.offset._42 = l_aibone->mOffsetMatrix.d2;
				m_bone.offset._43 = l_aibone->mOffsetMatrix.d3;
				m_bone.offset._44 = l_aibone->mOffsetMatrix.d4;


                for (unsigned int weightIndex = 0; weightIndex < l_aibone->mNumWeights; ++weightIndex) {
                    m_bone.weight = l_aibone->mWeights[weightIndex];

					m_bone.weight.mVertexId = l_aibone->mWeights->mVertexId;
					m_bone.weight.mWeight = l_aibone->mWeights->mWeight;

                    
                }
            }
        }
    }


}
// ボーンごとの最大数
constexpr int MAX_BONE_INFLUENCE = 4;
constexpr int MAX_BONES = 100;



// ボーン情報
struct BoneInfo {
    DirectX::XMMATRIX offsetMatrix;  // ボーンのオフセット行列
    DirectX::XMMATRIX finalTransform; // アニメーション計算後に渡す行列
};


std::map<std::string, int> boneMapping; // ボーン名 → インデックス
std::vector<BoneInfo> boneInfos;        // ボーンの情報配列
int boneCount = 0;                      // ボーンの総数



void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        aiBone* bone = mesh->mBones[boneIndex];
        std::string boneName(bone->mName.C_Str());

        int boneID = 0;
        if (boneMapping.find(boneName) == boneMapping.end()) {
            // 新規ボーン
            boneID = boneCount;
            boneMapping[boneName] = boneID;

            BoneInfo boneInfo;
            boneInfo.offsetMatrix = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&bone->mOffsetMatrix));
            boneInfos.push_back(boneInfo);
            ++boneCount;
        }
        else {
            boneID = boneMapping[boneName];
        }

        // 頂点にウェイトを割り当てる
        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
            unsigned int vertexID = bone->mWeights[weightIndex].mVertexId;
            float weight = bone->mWeights[weightIndex].mWeight;

            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
                if (vertices[vertexID].boneIDs[i] < 0) {
                    vertices[vertexID].boneIDs[i] = boneID;
                    vertices[vertexID].weights[i] = weight;
                    break;
                }
            }
        }
    }
}


for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[i];
    std::vector<Vertex> vertices;

    // 頂点などを読み込む処理（省略）

    ExtractBoneWeightForVertices(vertices, mesh, scene);
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

    // Vertex Buffer Description
    auto vertexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(DirectX::VertexPositionNormalColorTexture) * vertices.size(), // Total size
        D3D11_BIND_VERTEX_BUFFER,                                           // Bind as vertex buffer
        D3D11_USAGE_DYNAMIC,                                                // Dynamic usage
        D3D11_CPU_ACCESS_WRITE                                              // Allow CPU write access
    );

    // Initial data for Vertex Buffer
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();

    // Create Vertex Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBufferTemp;
    HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBufferTemp);
    if (FAILED(hr))
    {
        return hr;
    }

    // Store as ID3D11Resource
    m_vertexBuffer = vertexBufferTemp;

    // Index Buffer Description
    auto indexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(UINT) * indices.size(), // Total size
        D3D11_BIND_INDEX_BUFFER,       // Bind as index buffer
        D3D11_USAGE_DYNAMIC,           // Dynamic usage
        D3D11_CPU_ACCESS_WRITE         // Allow CPU write access
    );

    // Initial data for Index Buffer
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    // Create Index Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBufferTemp;
    hr = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBufferTemp);
    if (FAILED(hr))
    {
        return hr;
    }

    // Store as ID3D11Resource
    m_indexBuffer = indexBufferTemp;

    // Create Constant Buffer
    m_constantBuffer.Create(device);

	//m_boneBuffer.Create(device);

    //ボーンの座標変換
	m_bone.offset = DirectX::XMFLOAT4X4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

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
  /*
    // マテリアルプロパティ
    Material material = {};
    material.Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    material.Diffuse = DirectX::XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f); // 赤
    material.Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Shininess = 32.0f;
//    m_materialbuffer.Create(deviceResources->GetD3DDevice());
*/
   // m_materialbuffer.SetData(deviceResources->GetD3DDeviceContext(), material);

    return S_OK;
}

HRESULT education::Model::CreateTexture(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    ComPtr<ID3D11SamplerState> samplerState;
    HRESULT hr = device->CreateSamplerState(&samplerDesc, &samplerState);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create sampler state.\n");
        return hr;
    }
    hr = DirectX::CreateDDSTextureFromFile(device, L"SEAFLOOR.DDS",
        m_textureBuffer.ReleaseAndGetAddressOf(), m_modelsrv.ReleaseAndGetAddressOf(), 0, nullptr);
   
    return hr;
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





