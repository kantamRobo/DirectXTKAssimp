#pragma once
#include "Model.h"
#include <DirectXTex.h>
class TexturedModel:public education::Model
{
public:
	TexturedModel() {};
	TexturedModel(DX::DeviceResources* deviceresources, const char* path, int width, int height);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
public:
	HRESULT CreateTexture(ID3D11Device* device, const char* texturedPath);
	void DrawTextured(const DX::DeviceResources* DR);
private:
	

	

};

