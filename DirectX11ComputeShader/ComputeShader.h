#pragma once
#include <d3d11.h>

#include <wrl/client.h>
#include <DeviceResources.h>
#include "RWStructuredBuffer.h"

class ComputeShader
{
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer> outputbuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ReadbackBuffer;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> g_pFinalPassCS;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlobFinalPassCS;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav;

	rwStructuredBuffer RWbuffer = { 0, 0.0f };

	void CreateShader(ID3D11Device* device);
	void CreateComputeShader(ID3D11Device* pd3dDevice);
	void CreateBuffer(DX::DeviceResources* DR);
	void CreateUAV(ID3D11Device* device);
	void Dispatch(DX::DeviceResources* DR);
	void ReleaseResources();
};

