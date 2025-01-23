#pragma once
#include <DirectXMath.h>

class Camera {

public:
	Camera()
	{
		movetempx = 180;
		movetempy = 180;
		
		const float CAMERA_LOOK_AT_HEIGHT = 400.0f;
	
		CameraLookAtPosition = DirectX::XMFLOAT3(0, 0, 0);
	
		position = DirectX::XMLoadFloat3(&zero);
		temppos = position;
	}
	
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
	DirectX::XMFLOAT4X4 Modelmat;
	DirectX::XMFLOAT3 rotation;
	void Init(const DirectX::XMFLOAT3& playerpos, const DirectX::XMFLOAT3& enemypos);
	void Update(DirectX::XMFLOAT3 modelCenter, unsigned int width, unsigned int height);
	




	int movetempx = 180;
	int movetempy = 180;

	float moveX = 0;
	float moveY = 0;
	float  moveZ = 0;






	DirectX::XMVECTOR position; //ImguiÇ≈í≤êÆ

	DirectX::XMFLOAT3 target;

	DirectX::XMFLOAT3 CameraLookAtPosition;

	DirectX::XMVECTOR temppos = DirectX::XMLoadFloat3(&zero);

	

	DirectX::XMMATRIX GetProjectionMatrix() {
		return Projection;
	}
	DirectX::XMMATRIX GetViewMatrix (){
		return View;
	}

	DirectX::XMVECTOR GetCameraRight() {
		return right;
	}
	DirectX::XMVECTOR GetCameraForward() {
		return forward;
	}

	void SetForward(const DirectX::XMVECTOR& in)
	{
		forward = in;
	}

	void SetRight(const DirectX::XMVECTOR& in)
	{
		right = in;
	}
private:
	DirectX::XMVECTOR forward= DirectX::XMVectorSet(0,0, 0, 1);
	DirectX::XMVECTOR right = DirectX::XMVectorSet(1, 0, 0,0);
	DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -10.0f, 0.0f);
	DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR zero = DirectX::XMVectorSet(0, 0, 0,0);

};