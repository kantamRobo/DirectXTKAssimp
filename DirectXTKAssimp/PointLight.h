#pragma once
#include <DirectXMath.h>
#include <BufferHelpers.h>
// C++側のPointLight定義例
struct PointLight
{
    DirectX::XMFLOAT3 position;    // 位置ベクトル
    float intensity;               // 光の強さ

    DirectX::XMFLOAT3 color;       // 光の色（RGB）
    float range;                   // 光の有効範囲（減衰計算に利用）

    DirectX::XMFLOAT3 attenuation; // 減衰パラメータ (constant, linear, quadratic)
    float padding;                 // HLSL側で16バイト境界を保つためのパディング

    // コンストラクタ
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

        // ビュー変換
        DirectX::XMVECTOR posWorld = DirectX::XMLoadFloat3(&light.position);
        DirectX::XMVECTOR posView = DirectX::XMVector3Transform(posWorld, view);
        DirectX::XMStoreFloat3(&light.position, posView);
        lightbuffer.SetData(dr->GetD3DDeviceContext(), light);
    }

};