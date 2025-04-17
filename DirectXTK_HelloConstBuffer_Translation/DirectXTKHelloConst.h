#pragma once
#include <DirectXMath.h>
struct SceneConstantBuffer
{
    DirectX::XMFLOAT4 offset;
    float padding[60]; // Padding so the constant buffer is 256-byte aligned.
};

class DirectXTKHelloConst
{
public:
    DirectXTKHelloConst(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    struct SceneConstantBuffer
    {
        DirectX::XMFLOAT4 offset;
        float padding[60]; // Padding so the constant buffer is 256-byte aligned.
    };
    SceneConstantBuffer  m_constantBufferData;
};

// Update frame-based values.
void DirectXTKHelloConst::OnUpdate()
{
    const float translationSpeed = 0.005f;
    const float offsetBounds = 1.25f;

    m_constantBufferData.offset.x += translationSpeed;
    if (m_constantBufferData.offset.x > offsetBounds)
    {
        m_constantBufferData.offset.x = -offsetBounds;
    }
    memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
}

