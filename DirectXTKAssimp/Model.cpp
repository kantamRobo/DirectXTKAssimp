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



education::Model::Model(DX::DeviceResources* deviceresources,const char* path)
{
    if (!LoadModel(path))
    {
		std::abort();
    }
    
/*
�ǉ���
*/
    if (FAILED(CreateBuffer(deviceresources)))
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

HRESULT education::Model::CreateBuffer(DX::DeviceResources* deviceResources)
{
    // ���_�o�b�t�@�̍쐬
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(
            deviceResources->GetD3DDevice(),
            vertices.data(),
            static_cast<int>(vertices.size()),
            sizeof(DirectX::VertexPositionNormalColorTexture),
            D3D11_BIND_VERTEX_BUFFER,
            m_vertexBuffer.GetAddressOf()
        )
    );
    // �C���f�b�N�X�o�b�t�@�̍쐬
    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(
            deviceResources->GetD3DDevice(),
            indices.data(),
            static_cast<int>(indices.size()),
            sizeof(unsigned short),
            D3D11_BIND_INDEX_BUFFER,
            m_indexBuffer.GetAddressOf()
        )
    );
    return S_OK;
}
HRESULT education::Model::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //�p�C�v���C���X�e�[�g�̍쐬
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // ���_�V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
auto hr =D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
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
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

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

void education::Model::CreateBuffers(const DX::DeviceResources* deviceResources, Game* game)
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
    float    aspect = game->m_height / game->m_width;
    float    nearZ = 0.1f;
    float    farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    SceneCB cb;
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));

    m_constantBuffer.SetData(deviceResources->GetD3DDeviceContext(), cb);

}


HRESULT education::Model::craetepipelineState(const DX::DeviceResources* deviceResources)
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

	auto hr =deviceResources->GetD3DDevice()->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());


	if (FAILED(hr))
	{
		return hr;
	}

    return hr;
}





void education::Model::Draw(const DX::DeviceResources* DR) {
   

   
    
    if (vertices.empty() || indices.empty()) {
        return;
    }
    auto device = DR->GetD3DDevice();
    UINT size = sizeof(DirectX::VertexPositionNormalColorTexture);
	auto offset = 0u;
  auto context = DR->GetD3DDeviceContext();
  context->IASetInputLayout(m_modelInputLayout.Get());
  context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
  auto buffer = m_constantBuffer.GetBuffer();

  context->VSSetConstantBuffers(0, 1, &buffer);
  context->PSSetConstantBuffers(0, 1, &buffer);
  context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),&size, &offset);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
  context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
  context->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
}




