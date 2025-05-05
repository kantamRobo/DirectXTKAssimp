// Server.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

#include <wrl/client.h>
#include <vector>
#include <iostream>

using Microsoft::WRL::ComPtr;

// ──── ネットワークでやりとりする頂点フォーマット ─────────────────────────
struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

// ポートやテクスチャサイズは必要に応じて調整してください
static const UINT TEX_W = 800;
static const UINT TEX_H = 600;
static const USHORT PORT = 12345;

int main()
{
    // 1) Winsock 初期化
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return -1;
    }
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in srvaddr = {};
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = INADDR_ANY;
    srvaddr.sin_port = htons(PORT);
    bind(listenSock, (sockaddr*)&srvaddr, sizeof(srvaddr));
    listen(listenSock, SOMAXCONN);
    std::cout << "Waiting for client on port " << PORT << "...\n";
    SOCKET client = accept(listenSock, nullptr, nullptr);
    std::cout << "Client connected\n";

    // 2) D3D11 デバイス・コンテキスト作成 (オフスクリーン描画用)
    ComPtr<ID3D11Device>        device;
    ComPtr<ID3D11DeviceContext> ctx;
    D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &device, nullptr, &ctx
    );

    // 3) オフスクリーン用テクスチャとステージングテクスチャ
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = TEX_W;
    desc.Height = TEX_H;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    ComPtr<ID3D11Texture2D> rtTex;
    device->CreateTexture2D(&desc, nullptr, &rtTex);
    ComPtr<ID3D11RenderTargetView> rtv;
    device->CreateRenderTargetView(rtTex.Get(), nullptr, &rtv);

    // ステージングテクスチャは CPURead 用
    D3D11_TEXTURE2D_DESC stDesc = desc;
    stDesc.Usage = D3D11_USAGE_STAGING;
    stDesc.BindFlags = 0;
    stDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging;
    device->CreateTexture2D(&stDesc, nullptr, &staging);

    // 4) シンプル頂点／ピクセルシェーダーをコンパイル
    const char* vsCode =
        "struct VSIn { float3 pos:POSITION; float4 col:COLOR; };"
        "struct VSOut { float4 pos:SV_POSITION; float4 col:COLOR; };"
        "VSOut main(VSIn vin) { VSOut vout; vout.pos = float4(vin.pos,1); vout.col = vin.col; return vout; }";
    const char* psCode =
        "struct PSIn { float4 pos:SV_POSITION; float4 col:COLOR; };"
        "float4 main(PSIn pin):SV_TARGET { return pin.col; }";

    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;
    D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vsBlob, &errBlob);
    D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0, &psBlob, &errBlob);

    ComPtr<ID3D11VertexShader>   vs;
    ComPtr<ID3D11PixelShader>    ps;
    device->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), nullptr, &vs);
    device->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(), nullptr, &ps);

    // 入力レイアウト (POSITION + COLOR)
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    ComPtr<ID3D11InputLayout> inputLayout;
    device->CreateInputLayout(layoutDesc, 2,
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &inputLayout);

    // 5) メイン受信 → 描画 → 返送ループ
    while (true) {
        int vertexCount = 0;
        if (recv(client, (char*)&vertexCount, sizeof(vertexCount), MSG_WAITALL) <= 0)
            break;
        if (vertexCount <= 0) break;

        // 頂点バッファ受信
        std::vector<Vertex> verts(vertexCount);
        int bytes = vertexCount * sizeof(Vertex);
        int rec = 0;
        while (rec < bytes) {
            int r = recv(client, ((char*)verts.data()) + rec, bytes - rec, 0);
            if (r <= 0) break;
            rec += r;
        }
        if (rec < bytes) break;

        // D3D11 に頂点バッファを作成
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = bytes;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sd{ verts.data(), 0, 0 };
        ComPtr<ID3D11Buffer> vb;
        device->CreateBuffer(&bd, &sd, &vb);

        // パイプラインセットアップ & 描画
        UINT stride = sizeof(Vertex), offset = 0;
        ctx->IASetInputLayout(inputLayout.Get());
        ctx->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(vs.Get(), nullptr, 0);
        ctx->PSSetShader(ps.Get(), nullptr, 0);
        ctx->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
        float clearColor[4] = { 0.1f,0.1f,0.1f,1.0f };
        ctx->ClearRenderTargetView(rtv.Get(), clearColor);
        ctx->Draw(vertexCount, 0);

        // ステージングにコピー → CPU 読み出し
        ctx->CopyResource(staging.Get(), rtTex.Get());
        D3D11_MAPPED_SUBRESOURCE mapped;
        ctx->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        int rowPitch = mapped.RowPitch;
        int imgSize = rowPitch * TEX_H;
        std::vector<BYTE> img(imgSize);
        for (UINT y = 0; y < TEX_H; ++y) {
            memcpy(&img[y * rowPitch],
                (BYTE*)mapped.pData + y * rowPitch,
                rowPitch);
        }
        ctx->Unmap(staging.Get(), 0);

        // 画像データをホストに送信 (幅／高さ／行長／データサイズ＋本体)
        int w = TEX_W, h = TEX_H, pitch = rowPitch, dataSize = imgSize;
        send(client, (char*)&w, sizeof(w), 0);
        send(client, (char*)&h, sizeof(h), 0);
        send(client, (char*)&pitch, sizeof(pitch), 0);
        send(client, (char*)&dataSize, sizeof(dataSize), 0);
        send(client, (char*)img.data(), dataSize, 0);
    }

    closesocket(client);
    closesocket(listenSock);
    WSACleanup();
    return 0;
}
