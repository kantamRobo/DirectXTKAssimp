// Host.cpp
#include <winsock2.h>
#include <windows.h>

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

struct Vertex {
    float x, y, z;
    float r, g, b, a;
};

static const UINT TEX_W = 800;
static const UINT TEX_H = 600;
static const USHORT PORT = 12345;
static POINT g_ClientSize = { TEX_W, TEX_H };

ComPtr<ID3D11Device>        g_dev;
ComPtr<ID3D11DeviceContext> g_ctx;
ComPtr<IDXGISwapChain>      g_swap;
ComPtr<ID3D11RenderTargetView> g_rtv;
ComPtr<ID3D11InputLayout>      g_quadIL;
ComPtr<ID3D11Buffer>           g_quadVB;
ComPtr<ID3D11VertexShader>     g_quadVS;
ComPtr<ID3D11PixelShader>      g_quadPS;
ComPtr<ID3D11ShaderResourceView> g_texSRV;
ComPtr<ID3D11SamplerState>     g_samp;

// ホスト to サーバーの接続ソケット
SOCKET g_sock = INVALID_SOCKET;

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wp, lp);
}

// Direct3D11 + SwapChain の初期化
void InitD3D11(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = g_ClientSize.x;
    scd.BufferDesc.Height = g_ClientSize.y;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &g_swap, &g_dev, nullptr, &g_ctx
    );

    ComPtr<ID3D11Texture2D> backBuf;
    g_swap->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    g_dev->CreateRenderTargetView(backBuf.Get(), nullptr, &g_rtv);
}

// 画面全体にテクスチャを貼るためのフルスクリーンクアッド初期化
void InitQuadPipeline()
{
    // シェーダー
    const char* vsQ =
        "struct VSIn { float3 pos:POSITION; float2 uv:TEXCOORD; };"
        "struct VSOut { float4 pos:SV_POSITION; float2 uv:TEXCOORD; };"
        "VSOut main(VSIn vin){ VSOut o; o.pos = float4(vin.pos,1); o.uv=vin.uv; return o; }";
    const char* psQ =
        "Texture2D tex:register(t0); SamplerState smp:register(s0);"
        "float4 main(float2 uv:TEXCOORD):SV_TARGET{ return tex.Sample(smp, uv); }";

    ComPtr<ID3DBlob> vsB, psB, err;
    D3DCompile(vsQ, strlen(vsQ), nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vsB, &err);
    D3DCompile(psQ, strlen(psQ), nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0, &psB, &err);
    g_dev->CreateVertexShader(vsB->GetBufferPointer(), vsB->GetBufferSize(), nullptr, &g_quadVS);
    g_dev->CreatePixelShader(psB->GetBufferPointer(), psB->GetBufferSize(), nullptr, &g_quadPS);

    // 入力レイアウト (POSITION + TEXCOORD)
    D3D11_INPUT_ELEMENT_DESC ld[] = {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0, D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,   0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
    };
    g_dev->CreateInputLayout(ld, 2, vsB->GetBufferPointer(),
        vsB->GetBufferSize(), &g_quadIL);

    // フルスクリーンクアッド頂点
    struct QV { float x, y, z, u, v; };
    QV quadVerts[4] = {
        {-1,-1,0, 0,1}, {-1,1,0, 0,0},
        {1,-1,0,  1,1}, {1,1,0,  1,0}
    };
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(quadVerts);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sd{ quadVerts,0,0 };
    g_dev->CreateBuffer(&bd, &sd, &g_quadVB);

    // 動的テクスチャ + SRV + サンプラー
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = TEX_W; td.Height = TEX_H;
    td.MipLevels = 1; td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DYNAMIC;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ComPtr<ID3D11Texture2D> dynTex;
    g_dev->CreateTexture2D(&td, nullptr, &dynTex);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = td.Format;
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MipLevels = 1;
    g_dev->CreateShaderResourceView(dynTex.Get(), &srvd, &g_texSRV);

    D3D11_SAMPLER_DESC smpd = {};
    smpd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    smpd.AddressU = smpd.AddressV = smpd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    g_dev->CreateSamplerState(&smpd, &g_samp);
}

// サーバーから受け取った生 RGBA データをテクスチャに転送
void UpdateTexture(ID3D11Texture2D* dynTex, BYTE* data, int rowPitch)
{
    D3D11_MAPPED_SUBRESOURCE m;
    g_ctx->Map(dynTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
    for (UINT y = 0; y < TEX_H; ++y) {
        memcpy((BYTE*)m.pData + y * m.RowPitch,
            data + y * rowPitch,
            rowPitch);
    }
    g_ctx->Unmap(dynTex, 0);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd)
{
    // ウィンドウ生成
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"DXW";
    RegisterClass(&wc);
    RECT rc = { 0,0,(LONG)g_ClientSize.x,(LONG)g_ClientSize.y };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hWnd = CreateWindowW(wc.lpszClassName, L"Host Display",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInst, nullptr);
    ShowWindow(hWnd, nCmd);

    // D3D11 初期化
    InitD3D11(hWnd);
    InitQuadPipeline();

    // Winsock 接続
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in srv = {};
    srv.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);
    srv.sin_port = htons(PORT);
    connect(g_sock, (sockaddr*)&srv, sizeof(srv));

    // Example: 三角形頂点データをサーバーに送る
    std::vector<Vertex> tri = {
        { 0.0f, 0.5f, 0.0f, 1,0,0,1 },
        { 0.5f,-0.5f, 0.0f, 0,1,0,1 },
        {-0.5f,-0.5f, 0.0f, 0,0,1,1 }
    };
    int vc = (int)tri.size();
    send(g_sock, (char*)&vc, sizeof(vc), 0);
    send(g_sock, (char*)tri.data(), vc * sizeof(Vertex), 0);

    // 画像受信
    int w, h, pitch, dataSize;
    recv(g_sock, (char*)&w, sizeof(w), MSG_WAITALL);
    recv(g_sock, (char*)&h, sizeof(h), MSG_WAITALL);
    recv(g_sock, (char*)&pitch, sizeof(pitch), MSG_WAITALL);
    recv(g_sock, (char*)&dataSize, sizeof(dataSize), MSG_WAITALL);
    std::vector<BYTE> img(dataSize);
    int rec = 0;
    while (rec < dataSize) {
        int r = recv(g_sock, (char*)img.data() + rec, dataSize - rec, 0);
        if (r <= 0) break;
        rec += r;
    }

    // 動的テクスチャ更新
    // dynTex は InitQuadPipeline 内で作ったテクスチャ
    // （g_texSRV から元テクスチャを取得するか、グローバルに持つよう適宜変更してください）
    // ここでは便宜的に SRV から元のテクスチャをダウンキャスト
    ComPtr<ID3D11Resource> res;
    g_texSRV->GetResource(&res);
    auto dynTex2D = static_cast<ID3D11Texture2D*>(res.Get());
    UpdateTexture(dynTex2D, img.data(), pitch);

    // メインループ：１フレームだけ描画して終了
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // 描画
    g_ctx->OMSetRenderTargets(1, g_rtv.GetAddressOf(), nullptr);
    float clr[4] = { 0.2f,0.2f,0.2f,1 };
    g_ctx->ClearRenderTargetView(g_rtv.Get(), clr);

    UINT stride = sizeof(float) * 5, offset = 0;
    g_ctx->IASetInputLayout(g_quadIL.Get());
    g_ctx->IASetVertexBuffers(0, 1, g_quadVB.GetAddressOf(), &stride, &offset);
    g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    g_ctx->VSSetShader(g_quadVS.Get(), nullptr, 0);
    g_ctx->PSSetShader(g_quadPS.Get(), nullptr, 0);
    g_ctx->PSSetShaderResources(0, 1, g_texSRV.GetAddressOf());
    g_ctx->PSSetSamplers(0, 1, g_samp.GetAddressOf());

    g_ctx->Draw(4, 0);
    g_swap->Present(1, 0);

    // 後片付け
    closesocket(g_sock);
    WSACleanup();
    return 0;
}
