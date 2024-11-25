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

HRESULT education::Model::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //�p�C�v���C���X�e�[�g�̍쐬
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // ���_�V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
auto hr =D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), nullptr);
	if (FAILED(hr))
	{
		return hr;
	}

	// ���_�V�F�[�_�[�̍쐬
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}


    // �s�N�Z���V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        return hr;
    }

    //�s�N�Z���V�F�[�_�[�̍쐬
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
	// ���̓��C�A�E�g�̍쐬
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
    //���_�o�b�t�@�̍쐬
    DirectX::CreateStaticBuffer(deviceResources->GetD3DDevice(), vertices.data(), vertices.size(), sizeof(DirectX::VertexPositionNormalColorTexture), D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer.GetAddressOf());
    //�C���f�b�N�X�o�b�t�@�̍쐬
    DirectX::CreateStaticBuffer(deviceResources->GetD3DDevice(), indices.data(), indices.size(), sizeof(UINT), D3D11_BIND_INDEX_BUFFER, m_indexBuffer.GetAddressOf());
}


void education::Model::craetepipelineState(const DX::DeviceResources* deviceResources)
{

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    // ����
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
    assert(LoadModel(path), "���[�h�Ɏ��s");
    vertices = GenerateVertices();
}
