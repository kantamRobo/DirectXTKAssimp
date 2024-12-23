#pragma once
class GameObject
{
public:
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 localPosition = DirectX::XMFLOAT3 (0, 0, 0);

	//���[�J�����W�����߂�
	void SetLocalPosition(const DirectX::XMFLOAT3& origin)
	{

		localPosition = DirectX::XMFLOAT3(localPosition.x-origin.x, localPosition.y - origin.y, localPosition.z - origin.z);

	}

	void SetPosition(const DirectX::XMFLOAT3& v)
	{
		position = v;
	}




};

