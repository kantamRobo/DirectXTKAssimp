#include "pch.h"
#include "SkinnedModel.h"


// Assimp の 4x4 行列を DirectX::XMMATRIX (行優先) に変換
static DirectX::XMMATRIX ConvertMatrixToDirectX(const aiMatrix4x4& mat)
{
    return DirectX::XMMATRIX(
        mat.a1, mat.b1, mat.c1, mat.d1,
        mat.a2, mat.b2, mat.c2, mat.d2,
        mat.a3, mat.c3, mat.c3, mat.d3,
        mat.a4, mat.b4, mat.c4, mat.d4
    );
}

bool SkinnedModel::LoadFromFile(DX::DeviceResources* deviceResources, const std::string& filePath)
{
    Assimp::Importer importer;

    // DirectX(左手座標系)に合わせて読み込み。ボーンウェイトを最大4本に制限
    unsigned int flags = aiProcess_Triangulate |
        aiProcess_ConvertToLeftHanded |
        aiProcess_GenSmoothNormals |
        aiProcess_LimitBoneWeights;

    const aiScene* scene = importer.ReadFile(filePath, flags);
    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        return false;
    }

    std::vector<SkinnedVertex> vertices;
    std::vector<uint32_t> indices;

    boneMapping.clear();
    jointCount = 0;

    // シーン内の全メッシュを順次パースして1つのバッファに統合
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        ProcessMesh(scene->mMeshes[m], scene, vertices, indices);
    }

    indexCount = static_cast<uint32_t>(indices.size());
    finalJointMatrices.resize(jointCount, DirectX::XMMatrixIdentity());

    auto device = deviceResources->GetD3DDevice();

    // 1. 頂点バッファの作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(SkinnedVertex) * vertices.size());
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices.data();
    HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, m_VertexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    // 2. インデックスバッファの作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices.data();
    hr = device->CreateBuffer(&ibDesc, &ibData, m_IndexBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return CreateShaderResources(deviceResources);
}

void SkinnedModel::ProcessMesh(aiMesh* mesh, const aiScene* scene,
    std::vector<SkinnedVertex>& vertices,
    std::vector<uint32_t>& indices)
{
    uint32_t vertexBaseOffset = static_cast<uint32_t>(vertices.size());

    // 1. 頂点の基本情報（座標・法線・UV）の取得
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        SkinnedVertex vertex = {};
        vertex.Position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals()) {
            vertex.Normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoord = DirectX::XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        vertices.push_back(vertex);
    }

    // 2. ボーン情報・頂点ウェイトの抽出
    for (unsigned int b = 0; b < mesh->mNumBones; ++b) {
        aiBone* pBone = mesh->mBones[b];
        std::string boneName = pBone->mName.C_Str();
        uint32_t boneIndex = 0;

        if (boneMapping.find(boneName) == boneMapping.end()) {
            boneIndex = jointCount;
            jointCount++;

            BoneInfo bi = {};
            bi.id = boneIndex;
            // Assimp 行列を転置して DirectXMath の行優先規約に整合させる
            DirectX::XMStoreFloat4x4(&bi.offsetMatrix, ConvertMatrixToDirectX(pBone->mOffsetMatrix));
            boneMapping[boneName] = bi;
        }
        else {
            boneIndex = boneMapping[boneName].id;
        }

        // ボーンの影響を受ける頂点へウェイトとインデックスを格納
        for (unsigned int w = 0; w < pBone->mNumWeights; ++w) {
            uint32_t vertexId = vertexBaseOffset + pBone->mWeights[w].mVertexId;
            float weight = pBone->mWeights[w].mWeight;
            vertices[vertexId].AddBoneData(boneIndex, weight);
        }
    }

    // 3. ウェイトの正規化（念のため合計値を 1.0 に補正）
    for (size_t i = vertexBaseOffset; i < vertices.size(); ++i) {
        float sum = vertices[i].JointWeights.x + vertices[i].JointWeights.y +
            vertices[i].JointWeights.z + vertices[i].JointWeights.w;
        if (sum > 0.0f) {
            vertices[i].JointWeights.x /= sum;
            vertices[i].JointWeights.y /= sum;
            vertices[i].JointWeights.z /= sum;
            vertices[i].JointWeights.w /= sum;
        }
    }

    // 4. 面インデックスの取得
    for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
        aiFace face = mesh->mFaces[f];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(vertexBaseOffset + face.mIndices[j]);
        }
    }
}
bool SkinnedModel::CreateShaderResources(DX::DeviceResources* deviceResources)
{

    auto device = deviceResources->GetD3DDevice();
    auto context = deviceResources->GetD3DDeviceContext();
	//頂点シェーダーのコンパイル
    /*
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
    */
	auto createVetexShader = D3DCompileFromFile(L"SkinnedVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, m_VertexShaderBlob.GetAddressOf(), m_ErrorBlob.GetAddressOf());
    if(FAILED(createVetexShader))
    {
        if (m_ErrorBlob)
        {
            OutputDebugStringA((char*)m_ErrorBlob->GetBufferPointer());
        }
        return false;
	}
    auto createPixelShader = D3DCompileFromFile(L"SkinnedPixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, m_PixelShaderBlob.GetAddressOf(), m_ErrorBlob.GetAddressOf());
    if (FAILED(createPixelShader))
    {
        if (m_ErrorBlob)
        {
            OutputDebugStringA((char*)m_ErrorBlob->GetBufferPointer());
        }
        return false;
    }
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
     { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SkinnedVertex, Position),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SkinnedVertex, Normal),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(SkinnedVertex, TexCoord),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
     // uint32_t[4] に合わせて R32G32B32A32_UINT に指定
     { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(SkinnedVertex, JointIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 },
     { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(SkinnedVertex, JointWeights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };



	m_JointConstantBuffer = std::make_unique<DirectX::ConstantBuffer<JointConstantBuffer>>(device);
	m_JointConstantBuffer->Create(device);
    
	m_CameraConstantBuffer = std::make_unique<DirectX::ConstantBuffer<DirectX::XMFLOAT4X4>>(device);
	m_CameraConstantBuffer->Create(device);

        return true;
    }


    void SkinnedModel::Render(DX::DeviceResources* deviceResources)
    {
        auto context = deviceResources->GetD3DDeviceContext();
		 ID3D11Buffer* cameraBuffer = m_CameraConstantBuffer->GetBuffer();
		ID3D11Buffer* jointBuffer = m_JointConstantBuffer->GetBuffer();
        // 1. カメラ行列バッファ (View, Projection) をスロット 0 にバインド
        context->VSSetConstantBuffers(0, 1, &cameraBuffer);

        // 2. ボーン行列バッファ (JointMatrices) をスロット 1 にバインド
        context->VSSetConstantBuffers(1, 1, &jointBuffer);

        // 3. シェーダーのセット
        context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
        context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
        // 4. 描画コマンドの発行
        context->DrawIndexed(indexCount, 0, 0);

        //描画指令を送った後は一時リソースを解放
		cameraBuffer->Release();
		jointBuffer->Release();
	}