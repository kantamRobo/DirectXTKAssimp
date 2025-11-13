#include "pch.h"
#include "VertexTypes.h"
#include "DirectXTKHelloConstSceneRendering.h"


/*

**検討中の定数バッファの設定**

定数バッファの作成とデータ設定は正しいですが、`Draw`ではバッファの設定方法に誤りがあります。同じスロットで2つの異なるバッファ（シーンバッファと定数バッファ）をバインドしており、これが競合を引き起こしています。`OnUpdate`では定数バッファがスロット0で設定されていますが、`Draw`では再度スロット0にバインドしています。スロットを一意にし、競合を避ける必要があります。

**定数バッファの不一致と修正提案**

シーンバッファと定数バッファが間違って同じスロットにバインドされています。結局、最終的に使用されるのは最後にバインドされた定数バッファ（SceneConstantBuffer, offset）です。これにより、ワールド、ビュー、プロジェクション行列がHLSLで正しく扱われません。コードで2つの定数バッファ（PolyConstantBufferとSceneConstantBuffer）が使用されていますが、HLSLでは1つだけが定義されています。この不一致を解消するために、HLSL側で両方のcbufferを定義するべきです。

**定数バッファのバインディングエラー**

定数バッファのバインディングスロットの不一致が原因で、ワールドビュー投影行列などのデータがシェーダーに届いていません。HLSLとC++の定義が一致しないことも問題です。C++コードではオフセットバッファをb0にバインドしているのに対し、HLSLではワールドビュー投影バッファが必要です。解決策として、定数バッファを1つに統一するか、異なるスロットでバインドし、VSSetConstantBuffersを正しく呼ぶことが必要です。


思考時間: 25 秒


ご提示のコードを拝見すると、**ワールド／ビュー／プロジェクション行列用バッファ**と**オフセット用バッファ**の両方を、同じレジスタ（b0）にバインドしてしまっているために、行列データが上書きされ、シェーダには正しい変換行列が渡っていないのが原因です。

---

## 問題点

1. **同じスロットへの二重バインド**

   ```cpp
   // ここで scenebuffer（行列バッファ）を b0 にバインド…
   context->VSSetConstantBuffers(0, 1, &scenebuffer);
   context->PSSetConstantBuffers(0, 1, &scenebuffer);
   // …すぐに constantbuffer（オフセットバッファ）を同じ b0 にバインドして上書き**
   context->VSSetConstantBuffers(0, 1, &constantbuffer);
   context->PSSetConstantBuffers(0, 1, &constantbuffer);
   ```

   このように同じレジスタ (b0) に２回バインドすると、後の呼び出しで前のバッファが消えてしまい、シェーダはオフセットバッファだけを参照することになります。&#x20;

2. **HLSL 側 cbuffer 定義と C++ 側構造体の不整合**

   * HLSL には行列だけを受け取る `cbuffer PolyConstantBuffer`（World/View/Projection）しか定義がないのに、
   * C++ 側では行列用 (`SceneCB`) とオフセット用 (`SceneConstantBuffer`) の２つの異なる定数バッファを用意し、
   * それを両方ともレジスタ b0 にバインドしている。&#x20;

   → シェーダが期待するレイアウト（3×4×4 行列）と、実際に渡されるデータ（オフセットだけ）が食い違っています。

---

## 改善案

### 1) レジスタを分ける

HLSL 側で２つの cbuffer を明示的に定義し、別々のレジスタに割り当てます。

```hlsl
// HLSL：register(b0) に変換行列、register(b1) にオフセット
cbuffer TransformCB : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

cbuffer OffsetCB : register(b1)
{
    float4 offset;
    // 必要ならパディング
};
```

C++ 側では、それぞれを対応するスロットにバインドします。

```cpp
// 行列バッファを b0 に
context->VSSetConstantBuffers(0, 1, &scenebuffer);
context->PSSetConstantBuffers(0, 1, &scenebuffer);

// オフセットバッファを b1 に
context->VSSetConstantBuffers(1, 1, &constantbuffer);
context->PSSetConstantBuffers(1, 1, &constantbuffer);
```

### 2) １つの cbuffer にまとめる

行列とオフセットを同じ構造体にまとめ、HLSL 側でも １ つの cbuffer として扱う方法です。

```hlsl
cbuffer PolyConstantBuffer : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
    float4   offset;
    // （必要ならパディング）
};
```

C++ 側は `SceneCB` 構造体を拡張してオフセットを追加し、１ 回だけ b0 にバインドします。

```cpp
struct PolyCB
{
    XMFLOAT4X4 world;
    XMFLOAT4X4 view;
    XMFLOAT4X4 projection;
    XMFLOAT4   offset;
    // …パディング…
};

// 作成・更新後…
auto buffer = m_PolyBuffer.GetBuffer();
context->VSSetConstantBuffers(0, 1, &buffer);
context->PSSetConstantBuffers(0, 1, &buffer);
```

---

### まとめ

* **必ず HLSL の cbuffer 定義と C++ の `struct` が完全に一致** していること
* **異なる定数バッファは異なるレジスタにバインド** し、同じスロットで上書きしない

このいずれかの方法で修正すれば、ワールド／ビュー／プロジェクション行列とオフセットが正しくシェーダに渡り、座標変換後のポリゴンが描画できるようになるはずです。もしさらに詰まる点があればお知らせください。

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
    DR->GetD3DDeviceContext()->VSSetConstantBuffers(0, 1, &buffer); // ← これを追加

}





//シェーダー

HRESULT DirectXTKHelloConstSceneRendering::CreateShaders(const DX::DeviceResources* deviceResources)
{
    //パイプラインステートの作成
    auto device = deviceResources->GetD3DDevice();

    auto context = deviceResources->GetD3DDeviceContext();

    // 頂点シェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pVSBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> perrrorBlob;
    auto hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, pVSBlob.GetAddressOf(), perrrorBlob.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    // 頂点シェーダーの作成
    hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }


    // ピクセルシェーダーのコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> pPSBlob;
    hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, pPSBlob.GetAddressOf(), nullptr);

    if (FAILED(hr))
    {
        OutputDebugStringA(reinterpret_cast<const char*>(perrrorBlob->GetBufferPointer()));
        return hr;
    }

    //ピクセルシェーダーの作成
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
   



    state = std::make_unique<DirectX::CommonStates>(device);
    return hr;
}

HRESULT DirectXTKHelloConstSceneRendering::CreateBuffers(DX::DeviceResources* DR, int width, int height)
{

    vertices = {
     { DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // 赤
     { DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // 緑
     { DirectX::XMFLOAT3(0.0f,  0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }  // 青
    };

	// indices ={ 0, 1, 2 }; //CW
    indices = { 2,1,0 };//CCW


    auto device = DR->GetD3DDevice();
    auto context = DR->GetD3DDeviceContext();
   

  
    

  

    

   



    




    DX::ThrowIfFailed(
        (DirectX::CreateStaticBuffer(device,
            vertices.data(),
            vertices.size(),
            sizeof(DirectX::VertexPositionColor),
            D3D11_BIND_VERTEX_BUFFER, m_vertexBuffer.GetAddressOf())));








    DX::ThrowIfFailed(
        DirectX::CreateStaticBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, m_indexBuffer.GetAddressOf())
    );
    ;
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
    XMStoreFloat4x4(&cb.world,  XMMatrixTranspose (worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));
    m_SceneBuffer.Create(device);
    m_SceneBuffer.SetData(context, cb);
    m_constantBufferData.Create(device);





    m_constantBufferData.SetData(DR->GetD3DDeviceContext(), sceneCB);
    return S_OK;
}


void DirectXTKHelloConstSceneRendering::Draw(const DX::DeviceResources* DR) {
    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("Vertex or index buffer is empty.\n");
        return;
    }

    auto context = DR->GetD3DDeviceContext();

    // Input Layout 設定
    context->IASetInputLayout(m_modelInputLayout.Get());

    // インデックスバッファの設定
    auto indexBuffer = static_cast<ID3D11Buffer*>(m_indexBuffer.Get());
    // Draw() の中
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 頂点バッファの設定
    UINT stride = sizeof(DirectX::VertexPositionColor);
    UINT offset = 0;
    auto vertexBuffer = static_cast<ID3D11Buffer*>(m_vertexBuffer.Get());
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    // プリミティブトポロジー設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto constantbuffer = m_constantBufferData.GetBuffer();
    auto scenebuffer = m_SceneBuffer.GetBuffer();
    // 行列バッファを b0 に
    context->VSSetConstantBuffers(0, 1, &scenebuffer);
    context->PSSetConstantBuffers(0, 1, &scenebuffer);



    // シェーダー設定
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    //context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    auto RS = state->CullCounterClockwise();

    context->RSSetState(RS);

    //context->PSSetShaderResources(0, 1, m_modelsrv.GetAddressOf());
    // 描画コール   
    context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);
}
