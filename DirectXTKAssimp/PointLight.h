#pragma once
#include <DirectXMath.h>
#include <BufferHelpers.h>
// C++����PointLight��`��
struct PointLight
{
    DirectX::XMFLOAT3 position;    // �ʒu�x�N�g��
    float intensity;               // ���̋���

    DirectX::XMFLOAT3 color;       // ���̐F�iRGB�j
    float range;                   // ���̗L���͈́i�����v�Z�ɗ��p�j

    DirectX::XMFLOAT3 attenuation; // �����p�����[�^ (constant, linear, quadratic)
    float padding;                 // HLSL����16�o�C�g���E��ۂ��߂̃p�f�B���O

    // �R���X�g���N�^
    PointLight(
        const DirectX::XMFLOAT3& pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
        float inten = 1.0f,
        const DirectX::XMFLOAT3& col = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),
        float rng = 10.0f,
        const DirectX::XMFLOAT3& atten = DirectX::XMFLOAT3(1.0f, 0.09f, 0.032f))
        : position(pos),
        intensity(inten),
        color(col),
        range(rng),
        attenuation(atten),
        padding(0.0f)
    {
    }
};

class PointLightBufferGen
{
    DirectX::ConstantBuffer<PointLight> lightbuffer;
    PointLight light = {};
    void Init(DX::DeviceResources* dr)
    {
        lightbuffer.Create(dr->GetD3DDevice());

    }
    void TickLight(DX::DeviceResources* dr,const DirectX::XMMATRIX& view) {
        
        light.position = DirectX::XMFLOAT3(10.0f, 5.0f, -3.0f);
        light.intensity = 1.0f;
        light.color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
        light.range = 20.0f;
        light.attenuation = DirectX::XMFLOAT3(1.0f, 0.09f, 0.032f);

        // �r���[�ϊ�
        DirectX::XMVECTOR posWorld = DirectX::XMLoadFloat3(&light.position);
        DirectX::XMVECTOR posView = DirectX::XMVector3Transform(posWorld, view);
        DirectX::XMStoreFloat3(&light.position, posView);
        lightbuffer.SetData(dr->GetD3DDeviceContext(), light);
    }

};