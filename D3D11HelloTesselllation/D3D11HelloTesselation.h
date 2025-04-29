#pragma once
#include <wrl.h>
#include <d3dcompiler.h>
#include <DeviceResources.h>
#include <d3d11.h>
// 定数バッファ用構造体
struct CB_TESS
{
    float Inner;    // 内側テッセレーション係数
    float Outer;    // 外側テッセレーション係数
    float pad[2];
};
class D3D11HelloTessellelation
{
public:
    // シェーダーコンパイル
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, hsBlob, dsBlob, psBlob, errBlob;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>g_cbTessFactor;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    Microsoft::WRL::ComPtr<ID3D11HullShader> hs;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> ds;
    void Render(DX::DeviceResources* DR)
    {

        auto context = DR->GetD3DDeviceContext();
        // シェーダー／入力レイアウト設定
        context->IASetInputLayout(inputLayout.Get());
        // パッチリストで３頂点単位
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

        context->VSSetShader(vs.Get(), nullptr, 0);
        context->HSSetShader(hs.Get(), nullptr, 0);
        context->DSSetShader(ds.Get(), nullptr, 0);
        context->PSSetShader(ps.Get(), nullptr, 0);

        // 定数バッファにテッセ係数を書き込む
        CB_TESS cb;
        cb.Inner = 5.0f;   // 内側テッセ
        cb.Outer = 5.0f;   // 外側テッセ
        context->UpdateSubresource(g_cbTessFactor.Get(), 0, nullptr, &cb, 0, 0);
        // HS ステージにバインド
        context->HSSetConstantBuffers(0, 1, g_cbTessFactor.GetAddressOf());

        // 頂点バッファ設定
        UINT stride = sizeof(float) * 3;
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

        // ドロー
        context->Draw(3, 0);

        // リセット（次フレームのため）
        context->HSSetShader(nullptr, nullptr, 0);
        context->DSSetShader(nullptr, nullptr, 0);

    }
    HRESULT Init(DX::DeviceResources* DR) {
        auto device = DR->GetD3DDevice();
        
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG;
#endif
        // 同一ファイル内に各ステージが定義されているので、エントリで切り替え
        if (FAILED(D3DCompileFromFile(L"Tessellation.hlsl", nullptr, nullptr,
            "VSMain", "vs_5_0", flags, 0, &vsBlob, &errBlob))) return E_FAIL;
        if (FAILED(D3DCompileFromFile(L"Tessellation.hlsl", nullptr, nullptr,
            "HSMain", "hs_5_0", flags, 0, &hsBlob, &errBlob))) return E_FAIL;
        if (FAILED(D3DCompileFromFile(L"Tessellation.hlsl", nullptr, nullptr,
            "DSMain", "ds_5_0", flags, 0, &dsBlob, &errBlob))) return E_FAIL;
        if (FAILED(D3DCompileFromFile(L"Tessellation.hlsl", nullptr, nullptr,
            "PSMain", "ps_5_0", flags, 0, &psBlob, &errBlob))) return E_FAIL;

        // シェーダー生成
        device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vs.ReleaseAndGetAddressOf());
        device->CreateHullShader(hsBlob->GetBufferPointer(), hsBlob->GetBufferSize(), nullptr, hs.ReleaseAndGetAddressOf());
        device->CreateDomainShader(dsBlob->GetBufferPointer(), dsBlob->GetBufferSize(), nullptr, ds.ReleaseAndGetAddressOf());
        device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, ps.ReleaseAndGetAddressOf());

        // 入力レイアウト (VS の input)
        D3D11_INPUT_ELEMENT_DESC ie[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout(ie, _countof(ie),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            inputLayout.GetAddressOf());

        // 三角形の３頂点 (コントロールポイント)
        struct VTX { float x, y, z; };
        VTX tri[3] = {
            { -0.5f, 0.0f, 0.0f },
            {  0.5f, 0.0f, 0.0f },
            {  0.0f, 0.8f, 0.0f },
        };
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(tri);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sd = { tri };
        device->CreateBuffer(&bd, &sd, vertexBuffer.ReleaseAndGetAddressOf());

        // テッセ係数用定数バッファ
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CB_TESS);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        device->CreateBuffer(&bd, nullptr, g_cbTessFactor.ReleaseAndGetAddressOf());

        return S_OK;
    }


  

}
;