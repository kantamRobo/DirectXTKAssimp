// GameObject.h
#pragma once
#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
class GameObject;

class Transform
{
public:
    DirectX::XMFLOAT3 position = { 0, 0, 0 };
    DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
    DirectX::XMFLOAT3 scale = { 1, 1, 1 };

    Transform* parent = nullptr;
    std::vector<Transform*> children;

    void AddChild(Transform* child)
    {
        children.push_back(child);
        child->parent = this;
    }

    DirectX::XMMATRIX GetLocalMatrix() const;
    DirectX::XMMATRIX GetWorldMatrix() const;
};

class GameObject
{
public:
    Transform transform;

    void Update();
    void Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX viewProj);

    DirectX::XMMATRIX GetWorldMatrix() const { return m_worldMatrix; }

private:
    DirectX::XMMATRIX m_worldMatrix;
};


using namespace DirectX;

XMMATRIX Transform::GetLocalMatrix() const
{
    return XMMatrixScaling(scale.x, scale.y, scale.z) *
        XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
        XMMatrixTranslation(position.x, position.y, position.z);
}

XMMATRIX Transform::GetWorldMatrix() const
{
    if (parent)
        return GetLocalMatrix() * parent->GetWorldMatrix();
    else
        return GetLocalMatrix();
}

void GameObject::Update()
{
    m_worldMatrix = transform.GetWorldMatrix();
}

void GameObject::Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX viewProj)
{
    // 将来的にメッシュ描画などを追加
    // viewProjはシェーダーへ渡す際に利用
}