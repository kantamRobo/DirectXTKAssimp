#include "GameObject.h"


GameObject::GameObject(const DirectX::XMVECTOR& vec, GameObject* gameobj) :pos(vec)
{
	child = gameobj;
	mouse = std::make_unique<DirectX::Mouse>();
}


void GameObject::Tick()
{
	auto state = mouse->GetState();

	if (state.leftButton)
		// Left button is down

		XMFLOAT2 mousePosInPixels(float(m.x), float(m.y));
	if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0)
	{
		int mouseX, mouseY;
		GetMousePoint(&mouseX, &mouseY);
		this->pos.x = mouseX;
		this->pos.y = mouseY;

		child->pos = VSub(this->pos, child->pos);
	}
}