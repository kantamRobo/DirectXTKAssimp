#include "GameObject.h"
#include <DirectXMath.h>

GameObject::GameObject(const DirectX::XMVECTOR& vec, GameObject* gameobj) :pos(vec)
{
	parent = gameobj;
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


// ���[���h�s����v�Z���� 
DirectX::XMMATRIX GameObject::GetWorldMatrix() {

 // �X�P�[���A��]�A�ړ��̃��[�J���s����쐬MATRIX 
 
DirectX::XMMATRIX scalemat = DirectX::XMMatrixScaling(1,1,1);

 
 DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(rot, 0);
 DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslationFromVector(pos);


// ���[�J���ϊ��s��iS * R * T �̏��j MATRIX localMatrix = MMult(scaleMat, rotMat); localMatrix = MMult(localMatrix, transMat); 
// �e������ꍇ�͐e�̃��[���h�s��Ɗ|�����킹�� if (parent) { 
  // ���[�J���ϊ��s��iS * R * T �̏��j
 DirectX::XMMATRIX localMatrix = DirectX::XMMatrixMultiply(scalemat, rotMat);
 localMatrix = DirectX::XMMatrixMultiply(localMatrix, transMat);
 if (parent) {
 return DirectX::XMMatrixMultiply(parent->GetWorldMatrix(), localMatrix);
} else {

	 return localMatrix;


}

