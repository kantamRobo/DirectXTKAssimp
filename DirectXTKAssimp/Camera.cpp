#include "pch.h"
#include "Camera.h"



void Camera::Init(const DirectX::XMFLOAT3& modelCenter, const DirectX::XMFLOAT3& lookAtOffset) {
	const float CAMERA_DISTANCE = 1500.0f;  // カメラの距離を大きく

	position = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&modelCenter), DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(0, 0, -CAMERA_DISTANCE))));
	CameraLookAtPosition = modelCenter;
	SetCameraPositionAndTarget_UpVecY(position, CameraLookAtPosition);
	SetCameraNearFar(0.1f, 2000.0f);  // 視錐台をさらに広げる
}




void Camera::Update(DirectX::XMFLOAT3 modelCenter) {
	SetCameraNearFar(0.1f, 10000.0f);

	CameraLookAtPosition = modelCenter;

	// カメラ位置のデバッグ表示
	DrawFormatString(100, 20, GetColor(0, 255, 0), L"Camera Position: (%f, %f, %f)",
		position.x, position.y, position.z);
	DrawFormatString(100, 40, GetColor(0, 255, 0), L"LookAt Position: (%f, %f, %f)",
		CameraLookAtPosition.x, CameraLookAtPosition.y, CameraLookAtPosition.z);

	SetCameraPositionAndTarget_UpVecY(position, CameraLookAtPosition);
}


// Set up Matrices
DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -10.0f, 0.0f);
DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);

float fov = DirectX::XMConvertToRadians(45.0f);
float aspect = static_cast<float>(width) / static_cast<float>(height);
float nearZ = 0.1f;
float farZ = 100.0f;
DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);
