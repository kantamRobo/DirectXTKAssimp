#pragma once
#include <DirectXMath.h>
#include <Mouse.h>
#include "Model.h"
class GameObject
{
public:
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rot;
	DirectX::XMVECTOR scale;
	GameObject* parent;
	GameObject(const DirectX::XMVECTOR& pos, GameObject* gameobj);
	void Tick(DX::DeviceResources* DR);
	std::unique_ptr<DirectX::Mouse> mouse;
	education::Model model;
	
	DirectX::XMMATRIX GetWorldMatrix();
};