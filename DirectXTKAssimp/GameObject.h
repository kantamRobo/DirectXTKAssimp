#pragma once
#include <DirectXMath.h>
#include <Mouse.h>
class GameObject
{
public:
	DirectX::XMVECTOR pos;
	GameObject* child;
	GameObject(const DirectX::XMVECTOR& pos, GameObject* gameobj);
	std::unique_ptr<DirectX::Mouse> mouse;
	
	void Tick();
};