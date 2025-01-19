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


//TODO:�R���X�g���N�^�Ń��f�����[�h�E�o�b�t�@�쐬�E�V�F�[�_�[�쐬���s��



education::Model::Model(DX::DeviceResources* deviceresources,const char* path,int width,int height)
{
    if (!LoadModel(path))
    {
		std::abort();
    }
    
/*
�ǉ���
*/
    if (FAILED(CreateBuffers(deviceresources,width,height)))
    {
		std::abort();
    }
	if(FAILED(CreateShaders(deviceresources, L"VertexShader.hlsl", L"PixelShader.hlsl")))
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
    // �X�P�[���l��ݒ�
    float scaleFactor = 10.0f;


    for (int i = 0; i < vertices.size(); i++)
    {
        //��Z�@vertices[i].position = vertices[i].position,100.0f;
    }

    // ���_����0�łȂ����Ƃ��m�F
    assert(vertices.size() > 0 && "���_�̐����Ɏ��s");

    // �C���f�b�N�X����3�̔{���ł���A���C���f�b�N�X�����_���𒴂��Ȃ����Ƃ��m�F
    assert(indices.size() % 3 == 0 && "�C���f�b�N�X����3�̔{���łȂ�");
    assert(indices.size() <= vertices.size() * 3 && "�C���f�b�N�X�����_���𒴂��Ă��܂�");

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

        // �C���f�b�N�X�̐ݒ�
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
   
    // �{�[�������擾
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



HRESULT education::Model::CreateShaders(const DX::DeviceResources* deviceResources,const LPCTSTR& vertexshaderpath, const LPCTSTR&  pixelshaderpath)
{
    //�p�C�v���C���X�e�[�g�̍쐬
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // ���_�V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
auto hr =D3DCompileFromFile(vertexshaderpath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
		return hr;
	}

	// ���_�V�F�[�_�[�̍쐬
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
	if (FAILED(hr))
	{
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
		return hr;
	}


    // �s�N�Z���V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(pixelshaderpath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    //�s�N�Z���V�F�[�_�[�̍쐬
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
	if (FAILED(hr))
	{
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
		return hr;
	}
   
        // ���̓��C�A�E�g�̍쐬
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
    //���_�o�b�t�@�̍쐬
    DirectX::CreateStaticBuffer(deviceResources->GetD3DDevice(), vertices.data(), vertices.size(), sizeof(DirectX::VertexPositionNormalColorTexture), D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer.GetAddressOf());
    //�C���f�b�N�X�o�b�t�@�̍쐬
    DirectX::CreateStaticBuffer(deviceResources->GetD3DDevice(), indices.data(), indices.size(), sizeof(UINT), D3D11_BIND_INDEX_BUFFER, m_indexBuffer.GetAddressOf());
    //�萔�o�b�t�@�̍쐬
    m_constantBuffer.Create(deviceResources->GetD3DDevice());

    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);
    DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

    float    fov = DirectX::XMConvertToRadians(45.0f);
    float    aspect = height / width;
    float    nearZ = 0.1f;
    float    farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    SceneCB cb;
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
    context->IASetInputLayout(m_modelInputLayout.Get());
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    UINT size = sizeof(DirectX::VertexPositionNormalColorTexture);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &size, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto buffer = m_constantBuffer.GetBuffer();
    context->VSSetConstantBuffers(0, 1, &buffer);
    context->PSSetConstantBuffers(0, 1, &buffer);

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    context->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
}





