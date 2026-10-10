#pragma once
#include <d3d11.h>

#include <DeviceResources.h>
#include "RWStructuredBuffer.h"
class ComputeShader
{


public:
	ID3D11Buffer* outputbuffer = nullptr;
	ID3D11Buffer* ReadbackBuffer = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	void CreateShader(ID3D11Device* device);
	void CreateComputeShader(ID3D11Device* pd3dDevice);
	ID3D11ComputeShader* g_pFinalPassCS = NULL;
	ID3DBlob* pBlobFinalPassCS = nullptr;

	void CreateBuffer(DX::DeviceResources* DR);
	void CreateUAV(ID3D11Device* device);
	void Dispatch(DX::DeviceResources* DR);
	rwStructuredBuffer RWbuffer;
	ID3D11UnorderedAccessView* uav;
};

