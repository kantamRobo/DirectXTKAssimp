#pragma once
#include <DirectXMath.h>
#include <Mouse.h>
class GameObject
{
public:
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR rot;
	DirectX::XMVECTOR scale;
	GameObject* parent;
	GameObject(const DirectX::XMVECTOR& pos, GameObject* gameobj);
	std::unique_ptr<DirectX::Mouse> mouse;
	
	void Tick();
	DirectX::XMMATRIX GetWorldMatrix();
};