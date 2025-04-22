#pragma once
#include <d3d11.h>

#include "RWStructuredBuffer.h"
class ComputeShader
{
	

public:
	ID3D11Resource* buffer = nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	void CreateShader(ID3D11Device* device);
	void CreateComputeShader(ID3D11Device* pd3dDevice);
	void CreateBuffer();
	void CreateUAV(ID3D11Device* device);
	RWStructuredBuffer buffer;
};

