#include "GameObject.h"
#include <DirectXMath.h>

GameObject::GameObject(const DirectX::XMVECTOR& vec, GameObject* gameobj) :pos(vec)
{
	parent = gameobj;
	mouse = std::make_unique<DirectX::Mouse>();
}


void GameObject::Tick(DX::DeviceResources* DR)
{
	auto state = mouse->GetState();



	if (state.leftButton)
		// Left button is down

		XMFLOAT2 mousePosInPixels(float(m.x), float(m.y));
	if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0)
	{
		int mouseX, mouseY;
		GetMousePoint(&mouseX, &mouseY);
		this->pos = mouseX;
		this->pos.y = mouseY;

		this->pos = DirectX::XMVectorSubtract(parent->pos,this->pos);
	}

	model.Draw(DR);
}


// ワールド行列を計算する 
DirectX::XMMATRIX GameObject::GetWorldMatrix() {

 // スケール、回転、移動のローカル行列を作成MATRIX 
 
DirectX::XMMATRIX scalemat = DirectX::XMMatrixScaling(1,1,1);

 
 DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(rot, 0);
 DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslationFromVector(pos);


// ローカル変換行列（S * R * T の順） MATRIX localMatrix = MMult(scaleMat, rotMat); localMatrix = MMult(localMatrix, transMat); 
// 親がいる場合は親のワールド行列と掛け合わせる if (parent) { 
  // ローカル変換行列（S * R * T の順）
 DirectX::XMMATRIX localMatrix = DirectX::XMMatrixMultiply(scalemat, rotMat);
 localMatrix = DirectX::XMMatrixMultiply(localMatrix, transMat);
 if (parent) {
 return DirectX::XMMatrixMultiply(parent->GetWorldMatrix(), localMatrix);
} else {

	 return localMatrix;


}

