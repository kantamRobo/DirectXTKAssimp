#include "pch.h"
#include "VertexTypes.h"
#include "DirectXTKHelloConstSceneRendering.h"


/*

**�������̒萔�o�b�t�@�̐ݒ�**

�萔�o�b�t�@�̍쐬�ƃf�[�^�ݒ�͐������ł����A`Draw`�ł̓o�b�t�@�̐ݒ���@�Ɍ�肪����܂��B�����X���b�g��2�̈قȂ�o�b�t�@�i�V�[���o�b�t�@�ƒ萔�o�b�t�@�j���o�C���h���Ă���A���ꂪ�����������N�����Ă��܂��B`OnUpdate`�ł͒萔�o�b�t�@���X���b�g0�Őݒ肳��Ă��܂����A`Draw`�ł͍ēx�X���b�g0�Ƀo�C���h���Ă��܂��B�X���b�g����ӂɂ��A�����������K�v������܂��B

**�萔�o�b�t�@�̕s��v�ƏC�����**

�V�[���o�b�t�@�ƒ萔�o�b�t�@���Ԉ���ē����X���b�g�Ƀo�C���h����Ă��܂��B���ǁA�ŏI�I�Ɏg�p�����͍̂Ō�Ƀo�C���h���ꂽ�萔�o�b�t�@�iSceneConstantBuffer, offset�j�ł��B����ɂ��A���[���h�A�r���[�A�v���W�F�N�V�����s��HLSL�Ő����������܂���B�R�[�h��2�̒萔�o�b�t�@�iPolyConstantBuffer��SceneConstantBuffer�j���g�p����Ă��܂����AHLSL�ł�1��������`����Ă��܂��B���̕s��v���������邽�߂ɁAHLSL���ŗ�����cbuffer���`����ׂ��ł��B

**�萔�o�b�t�@�̃o�C���f�B���O�G���[**

�萔�o�b�t�@�̃o�C���f�B���O�X���b�g�̕s��v�������ŁA���[���h�r���[���e�s��Ȃǂ̃f�[�^���V�F�[�_�[�ɓ͂��Ă��܂���BHLSL��C++�̒�`����v���Ȃ����Ƃ����ł��BC++�R�[�h�ł̓I�t�Z�b�g�o�b�t�@��b0�Ƀo�C���h���Ă���̂ɑ΂��AHLSL�ł̓��[���h�r���[���e�o�b�t�@���K�v�ł��B������Ƃ��āA�萔�o�b�t�@��1�ɓ��ꂷ�邩�A�قȂ�X���b�g�Ńo�C���h���AVSSetConstantBuffers�𐳂����ĂԂ��Ƃ��K�v�ł��B


�v�l����: 25 �b


���񎦂̃R�[�h��q������ƁA**���[���h�^�r���[�^�v���W�F�N�V�����s��p�o�b�t�@**��**�I�t�Z�b�g�p�o�b�t�@**�̗������A�������W�X�^�ib0�j�Ƀo�C���h���Ă��܂��Ă��邽�߂ɁA�s��f�[�^���㏑������A�V�F�[�_�ɂ͐������ϊ��s�񂪓n���Ă��Ȃ��̂������ł��B

---

## ���_

1. **�����X���b�g�ւ̓�d�o�C���h**

   ```cpp
   // ������ scenebuffer�i�s��o�b�t�@�j�� b0 �Ƀo�C���h�c
   context->VSSetConstantBuffers(0, 1, &scenebuffer);
   context->PSSetConstantBuffers(0, 1, &scenebuffer);
   // �c������ constantbuffer�i�I�t�Z�b�g�o�b�t�@�j�𓯂� b0 �Ƀo�C���h���ď㏑��**
   context->VSSetConstantBuffers(0, 1, &constantbuffer);
   context->PSSetConstantBuffers(0, 1, &constantbuffer);
   ```

   ���̂悤�ɓ������W�X�^ (b0) �ɂQ��o�C���h����ƁA��̌Ăяo���őO�̃o�b�t�@�������Ă��܂��A�V�F�[�_�̓I�t�Z�b�g�o�b�t�@�������Q�Ƃ��邱�ƂɂȂ�܂��B&#x20;

2. **HLSL �� cbuffer ��`�� C++ ���\���̂̕s����**

   * HLSL �ɂ͍s�񂾂����󂯎�� `cbuffer PolyConstantBuffer`�iWorld/View/Projection�j������`���Ȃ��̂ɁA
   * C++ ���ł͍s��p (`SceneCB`) �ƃI�t�Z�b�g�p (`SceneConstantBuffer`) �̂Q�̈قȂ�萔�o�b�t�@��p�ӂ��A
   * ����𗼕��Ƃ����W�X�^ b0 �Ƀo�C���h���Ă���B&#x20;

   �� �V�F�[�_�����҂��郌�C�A�E�g�i3�~4�~4 �s��j�ƁA���ۂɓn�����f�[�^�i�I�t�Z�b�g�����j���H������Ă��܂��B

---

## ���P��

### 1) ���W�X�^�𕪂���

HLSL ���łQ�� cbuffer �𖾎��I�ɒ�`���A�ʁX�̃��W�X�^�Ɋ��蓖�Ă܂��B

```hlsl
// HLSL�Fregister(b0) �ɕϊ��s��Aregister(b1) �ɃI�t�Z�b�g
cbuffer TransformCB : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

cbuffer OffsetCB : register(b1)
{
    float4 offset;
    // �K�v�Ȃ�p�f�B���O
};
```

C++ ���ł́A���ꂼ���Ή�����X���b�g�Ƀo�C���h���܂��B

```cpp
// �s��o�b�t�@�� b0 ��
context->VSSetConstantBuffers(0, 1, &scenebuffer);
context->PSSetConstantBuffers(0, 1, &scenebuffer);

// �I�t�Z�b�g�o�b�t�@�� b1 ��
context->VSSetConstantBuffers(1, 1, &constantbuffer);
context->PSSetConstantBuffers(1, 1, &constantbuffer);
```

### 2) �P�� cbuffer �ɂ܂Ƃ߂�

�s��ƃI�t�Z�b�g�𓯂��\���̂ɂ܂Ƃ߁AHLSL ���ł� �P �� cbuffer �Ƃ��Ĉ������@�ł��B

```hlsl
cbuffer PolyConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4   offset;
    // �i�K�v�Ȃ�p�f�B���O�j
};
```

C++ ���� `SceneCB` �\���̂��g�����ăI�t�Z�b�g��ǉ����A�P �񂾂� b0 �Ƀo�C���h���܂��B

```cpp
struct PolyCB
{
    XMFLOAT4X4 world;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
    XMFLOAT4   offset;
    // �c�p�f�B���O�c
};

// �쐬�E�X�V��c
auto buffer = m_PolyBuffer.GetBuffer();
context->VSSetConstantBuffers(0, 1, &buffer);
context->PSSetConstantBuffers(0, 1, &buffer);
```

---

### �܂Ƃ�

* **�K�� HLSL �� cbuffer ��`�� C++ �� `struct` �����S�Ɉ�v** ���Ă��邱��
* **�قȂ�萔�o�b�t�@�͈قȂ郌�W�X�^�Ƀo�C���h** ���A�����X���b�g�ŏ㏑�����Ȃ�

���̂����ꂩ�̕��@�ŏC������΁A���[���h�^�r���[�^�v���W�F�N�V�����s��ƃI�t�Z�b�g���������V�F�[�_�ɓn��A���W�ϊ���̃|���S�����`��ł���悤�ɂȂ�͂��ł��B��������ɋl�܂�_������΂��m�点���������B

*/


DirectXTKHelloConstSceneRendering::DirectXTKHelloConstSceneRendering(UINT width, UINT height, std::wstring name)
{
}

// Update frame-based values.
void DirectXTKHelloConstSceneRendering::OnUpdate(DX::DeviceResources* DR)
{
    const float translationSpeed = 0.005f;
    const float offsetBounds = 1.25f;


    sceneCB.offset.x += translationSpeed;

    if (sceneCB.offset.x > offsetBounds)
    {
        sceneCB.offset.x = -offsetBounds;
    }
    m_constantBufferData.SetData(DR->GetD3DDeviceContext(), sceneCB);
    auto buffer = m_constantBufferData.GetBuffer();
    DR->GetD3DDeviceContext()->PSSetConstantBuffers(0, 1, &buffer);
    DR->GetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &buffer); // �� �����ǉ�

}



void DirectXTKHelloConstSceneRendering::Draw(const DX::DeviceResources* DR) {
    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("Vertex or index buffer is empty.\n");
        return;
    }

    auto context = DR->GetD3DDeviceContext();

    // Input Layout �ݒ�
    context->IASetInputLayout(m_modelInputLayout.Get());

    // �C���f�b�N�X�o�b�t�@�̐ݒ�
    auto indexBuffer = static_cast<ID3D11Buffer*>(m_indexBuffer.Get());
    // Draw() �̒�
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // ���_�o�b�t�@�̐ݒ�
    UINT stride = sizeof(DirectX::VertexPositionColor);
    UINT offset = 0;
    auto vertexBuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    // �v���~�e�B�u�g�|���W�[�ݒ�
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto constantbuffer = m_constantBufferData.GetBuffer();
    auto scenebuffer = m_SceneBuffer.GetBuffer();
    // �s��o�b�t�@�� b0 ��
    context->VSSetConstantBuffers(0, 1, &scenebuffer);
    context->PSSetConstantBuffers(0, 1, &scenebuffer);

    // �I�t�Z�b�g�o�b�t�@�� b1 ��
    context->VSSetConstantBuffers(1, 1, &constantbuffer);
    context->PSSetConstantBuffers(1, 1, &constantbuffer);


    // �V�F�[�_�[�ݒ�
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    //context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
    context->RSSetState(m_rasterizerState.Get());

    //context->PSSetShaderResources(0, 1, m_modelsrv.GetAddressOf());
    // �`��R�[��   
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}


//�V�F�[�_�[

HRESULT DirectXTKHelloConstSceneRendering::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //�p�C�v���C���X�e�[�g�̍쐬
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // ���_�V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
    auto hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    // ���_�V�F�[�_�[�̍쐬
    hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }


    // �s�N�Z���V�F�[�_�[�̃R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    //�s�N�Z���V�F�[�_�[�̍쐬
    hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    UINT numElements = ARRAYSIZE(layout);

    hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_modelInputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        return hr;
    }
    if (!m_rasterizerState)
    {
        D3D11_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_NONE; // �� �o�b�N�t�F�C�X�J�����O���ꎞ�I�ɃI�t��
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.DepthClipEnable = true;

        device->CreateRasterizerState(&rasterDesc, &m_rasterizerState);
    }




    return hr;
}

HRESULT DirectXTKHelloConstSceneRendering::CreateBuffers(DX::DeviceResources* DR, int width, int height)
{

    vertices = {
     { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // ��
     { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // ��
     { DirectX::XMFLOAT3(0.0f,  0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }  // ��
    };

    indices = { 0, 1, 2 };



    auto device = DR->GetD3DDevice();
    auto context = DR->GetD3DDeviceContext();
    // Vertex Buffer Description
    auto vertexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(DirectX::VertexPositionColor) * vertices.size(), // Total size
        D3D11_BIND_VERTEX_BUFFER,                                           // Bind as vertex buffer
        D3D11_USAGE_DYNAMIC,                                                // Dynamic usage
        D3D11_CPU_ACCESS_WRITE                                              // Allow CPU write access
    );


    // Initial data for Vertex Buffer
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();

    // Create Vertex Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBufferTemp;
    HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBufferTemp);
    if (FAILED(hr))
    {
        return hr;
    }

    // Store as ID3D11Resource
    m_vertexBuffer = vertexBufferTemp;

    // Index Buffer Description
    auto indexBufferDesc = CD3D11_BUFFER_DESC(
        sizeof(UINT) * indices.size(), // Total size
        D3D11_BIND_INDEX_BUFFER,       // Bind as index buffer
        D3D11_USAGE_DYNAMIC,           // Dynamic usage
        D3D11_CPU_ACCESS_WRITE         // Allow CPU write access
    );

    // Initial data for Index Buffer
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();

    // Create Index Buffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBufferTemp;
    hr = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBufferTemp);
    if (FAILED(hr))
    {
        return hr;
    }

    // Store as ID3D11Resource
    m_indexBuffer = indexBufferTemp;
    // Update Constant Buffer  // Set up Matrices
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

    // Update Constant Buffer
    SceneCB cb = {};
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));
    m_SceneBuffer.Create(device);
    m_SceneBuffer.SetData(context, cb);
    m_constantBufferData.Create(device);





    m_constantBufferData.SetData(DR->GetD3DDeviceContext(), sceneCB);
    return S_OK;
}
