#pragma once
#include <DirectXMath.h>

class Camera {

public:
	Camera()
	{
		movetempx = 180;
		movetempy = 180;
		forward = DirectX::XMFLOAT3(1, 0, 0);
		right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
		const float CAMERA_LOOK_AT_HEIGHT = 400.0f;

		CameraLookAtPosition = DirectX::XMFLOAT3(0, 0, 0);
		position = DirectX::XMFLOAT3(0, 0, 0);
		temppos = position;
	}
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4X4 Modelmat;
	DirectX::XMFLOAT3 rotation;
	void Init(const DirectX::XMFLOAT3& playerpos, const DirectX::XMFLOAT3& enemypos);
	void Update(DirectX::XMFLOAT3 modelCenter);




	int movetempx = 180;
	int movetempy = 180;

	float moveX = 0;
	float moveY = 0;
	float  moveZ = 0;






	DirectX::XMVECTOR position; //Imguiで調整

	DirectX::XMFLOAT3 target;

	DirectX::XMFLOAT3 CameraLookAtPosition;

	DirectX::XMFLOAT3 temppos = DirectX::XMFLOAT3(0,0,0);


	DirectX::XMFLOAT4X4 GetProjectionMatrix() {
		return Projection;
	}
	DirectX::XMFLOAT4X4 GetViewMatrix (){
		return View;
	}

	DirectX::XMFLOAT3 GetCameraRight() {
		return right;
	}
	DirectX::XMFLOAT3 GetCameraForward() {
		return forward;
	}

	void SetForward(const DirectX::XMFLOAT3& in)
	{
		forward = in;
	}

	void SetRight(const DirectX::XMFLOAT3& in)
	{
		right = in;
	}
private:
	DirectX::XMFLOAT3 forward= DirectX::XMFLOAT3(0, 0, 1);
	DirectX::XMFLOAT3 right = DirectX::XMFLOAT3(1, 0, 0);
private:
	// カメラの向きを更新する (例えば回転行列を使って計算する)

};