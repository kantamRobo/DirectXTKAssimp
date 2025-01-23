#pragma once
#include "pch.h"
#include <array>
#include <DeviceResources.h>
#include <d3dcompiler.h>
class Triangle
{
public:
	std::array < DirectX::VertexPositionNormalColorTexture, 3 > vertices;
	
	Triangle(DirectX::VertexPositionNormalColorTexture in_vertpos1, DirectX::VertexPositionNormalColorTexture in_vertpos2, DirectX::VertexPositionNormalColorTexture in_vertpos3)
	{

		vertices[0] = in_vertpos1;
		vertices[1] = in_vertpos2;
		vertices[2] = in_vertpos3;



	}
	Microsoft::WRL::ComPtr<ID3D11Buffer> vb;


	HRESULT Init(DX::DeviceResources* dr,unsigned int width,unsigned int height)
	{
		//パイプラインステートの作成
		auto device = dr->GetD3DDevice();


		auto context = dr->GetD3DDeviceContext();

		
		auto hr = DirectX::CreateStaticBuffer(device,
			vertices.data(),                // const void *ptr
			vertices.size(),     // size_t count
			sizeof(DirectX::VertexPositionNormalColorTexture), // size_t stride
			D3D11_BIND_VERTEX_BUFFER, &vb);
			if (FAILED(hr))
			{
				return hr;
		}

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

			m_constantBuffer.SetData(context, cb);
		

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


		auto hr = device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());

		
		// 頂点シェーダーのコンパイル
		Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
		auto hr = D3DCompileFromFile(L"PolygonVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
		if (FAILED(hr))
		{
			OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
			return hr;
		}

		// 頂点シェーダーの作成
		hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_PolygonvertexShader.GetAddressOf());
		if (FAILED(hr))
		{
			OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
			return hr;
		}


		// ピクセルシェーダーのコンパイル
		Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
		hr = D3DCompileFromFile(L"PolygonPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

		if (FAILED(hr))
		{
			OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
			return hr;
		}

		//ピクセルシェーダーの作成
		hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_PolygonpixelShader.GetAddressOf());
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
			//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	void Draw()
	{
		
	}

private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	//シェーダーの作成
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_PolygonvertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PolygonpixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_modelInputLayout = nullptr;// 入力レイアウト
	DirectX::ConstantBuffer<SceneCB> m_constantBuffer;
};

