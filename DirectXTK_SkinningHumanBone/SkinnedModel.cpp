#include "pch.h"
#include "SkinnedModel.h"



bool SkinnedModel::CreateShaderResources(DX::DeviceResources* deviceResources)
{

    auto device = deviceResources->GetD3DDevice();
    auto context = deviceResources->GetD3DDeviceContext();

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
    { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SkinnedVertex, Position),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(SkinnedVertex, Normal),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(SkinnedVertex, TexCoord),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
    // 関節インデックス (uint4)
    { "BLENDINDICES", 0, DXGI_FORMAT_R16_UINT,   0, offsetof(SkinnedVertex, JointIndices), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    // 関節ウェイト (float4)
    { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, offsetof(SkinnedVertex, JointWeights), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };




    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(JointConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    auto JOINTBUFFER = device->CreateBuffer(&cbDesc, nullptr, pJointCBuffer.GetAddressOf());
    if (FAILED(JOINTBUFFER))
    {
        return false;
    }
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(pJointCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        JointConstantBuffer* pData = reinterpret_cast<JointConstantBuffer*>(mappedResource.pData);
        for (int i = 0; i < jointCount; ++i) {
            // DirectXMath は行優先(Row-Major)、HLSL のデフォルトは列優先(Column-Major)のため
            // 必要に応じて転置 (Transpose) を行って格納します
            DirectX::XMStoreFloat4x4(&pData->JointMatrices[i], DirectX::XMMatrixTranspose(finalJointMatrices[i]));
        }
        context->Unmap(pJointCBuffer.Get(), 0);

        return true;
    }
}

    void SkinnedModel::Render(DX::DeviceResources* deviceResources)
    {
        auto context = deviceResources->GetD3DDeviceContext();
        // 1. カメラ行列バッファ (View, Projection) をスロット 0 にバインド
        context->VSSetConstantBuffers(0, 1, pCameraCBuffer.GetAddressOf());

        // 2. ボーン行列バッファ (JointMatrices) をスロット 1 にバインド
        context->VSSetConstantBuffers(1, 1, pJointCBuffer.GetAddressOf());

        // 3. シェーダーのセット
        context->VSSetShader(pVertexShader.Get(), nullptr, 0);
        context->PSSetShader(pPixelShader.Get(), nullptr, 0);

        // 4. 描画コマンドの発行
        context->DrawIndexed(indexCount, 0, 0);


	}