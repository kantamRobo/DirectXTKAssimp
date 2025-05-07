#include <winsock2.h>
#include <ws2tcpip.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

static const int WIDTH = 800;
static const int HEIGHT = 600;



// クライアントから受け取る頂点
struct Vertex {
    float x, y, z;
};


int main() {
//ここが違う {
    // --- Winsock2 初期化 & 接続待ち ---
    WSADATA wsa;
if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    MessageBox(nullptr, L"WSAStartup failed", L"Error", MB_OK);
    return -1;
}
SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
sockaddr_in addr = {};
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(9000);
bind(listenSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
listen(listenSock, SOMAXCONN);
OutputDebugString(L"Server listening on port 9000\n");

SOCKET client = accept(listenSock, nullptr, nullptr);
OutputDebugString(L"Client connected\n");

// --- 頂点データ受信 & デバッグ出力 ---
int count = 0;
recv(client, reinterpret_cast<char*>(&count), sizeof(count), MSG_WAITALL);
{
    char buf[64];
    sprintf_s(buf, "Received vertex count: %d\n", count);
    //OutputDebugString(buf);
}

Vertex* vertices = new Vertex[count];
recv(client, reinterpret_cast<char*>(vertices), count * sizeof(Vertex), MSG_WAITALL);

// --- Win32 ウィンドウ作成 ---
WNDCLASS wc = {};
wc.lpfnWndProc = WndProc;
wc.hInstance = hInst;
wc.lpszClassName = L"DX11ServerWnd";
RegisterClass(&wc);
HWND hWnd = CreateWindow(wc.lpszClassName, L"Server Renderer",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
    nullptr, nullptr, hInst, nullptr);
ShowWindow(hWnd, nCmd);
//改造後ではオフスクリーン用テクスチャ
// 
// オフスクリーン用テクスチャ + RTV【追加】
ID3D11Texture2D* renderTex = nullptr;
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = WIDTH;
    desc.Height = HEIGHT;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    dev->CreateTexture2D(&desc, nullptr, &renderTex);
}

//+RTVの初期化が追加されている
// ―― D3D11 デバイス作成（オフスクリーン用）―― 【変更】
ID3D11Device* dev = nullptr;
ID3D11DeviceContext* ctx = nullptr;
if (FAILED(D3D11CreateDevice(
    nullptr, D3D_DRIVER_TYPE_HARDWARE,
    nullptr, 0, nullptr, 0,
    D3D11_SDK_VERSION, &dev, nullptr, &ctx))) {
    return -1;
}

// オフスクリーン用テクスチャ + RTV【追加】
ID3D11Texture2D* renderTex = nullptr;
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = WIDTH;
    desc.Height = HEIGHT;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    dev->CreateTexture2D(&desc, nullptr, &renderTex);
}
ID3D11RenderTargetView* offRtv = nullptr;
dev->CreateRenderTargetView(renderTex, nullptr, &offRtv);
// Map→CPU読み出し用 Stagingテクスチャ【追加】
ID3D11Texture2D* stagingTex = nullptr;
{
    D3D11_TEXTURE2D_DESC desc = {};
    renderTex->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    dev->CreateTexture2D(&desc, nullptr, &stagingTex);
}

// ビューポート設定【追加】
D3D11_VIEWPORT vp = {};
vp.Width = (FLOAT)WIDTH;
vp.Height = (FLOAT)HEIGHT;
vp.MinDepth = 0.0f;
vp.MaxDepth = 1.0f;
ctx->RSSetViewports(1, &vp);


//改造後ではMap→CPU読み出し用Staging
//テクスチャの追加
//改造後はビューポート設定が追加されている

    // --- D3D11 初期化 ---
    DXGI_SWAP_CHAIN_DESC scd = {};
scd.BufferCount = 1;
scd.BufferDesc.Width = 800;
scd.BufferDesc.Height = 600;
scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
scd.OutputWindow = hWnd;
scd.SampleDesc.Count = 1;
scd.Windowed = TRUE;

ID3D11Device* dev = nullptr;
ID3D11DeviceContext* ctx = nullptr;
IDXGISwapChain* swap = nullptr;
HRESULT hr = D3D11CreateDeviceAndSwapChain(
    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
    nullptr, 0, D3D11_SDK_VERSION,
    &scd, &swap, &dev, nullptr, &ctx
);
if (FAILED(hr)) {
    OutputDebugString(L"D3D11CreateDeviceAndSwapChain failed\n");
    return -1;
}

// レンダーターゲットビュー作成
ID3D11Texture2D* backBuf = nullptr;
swap->GetBuffer(0, IID_PPV_ARGS(&backBuf));
ID3D11RenderTargetView* rtv = nullptr;
dev->CreateRenderTargetView(backBuf, nullptr, &rtv);
//backBuf->Release();
// ―― ここを追加 ――
// レンダーターゲットを出力マージステージにバインド
ctx->OMSetRenderTargets(1, &rtv, nullptr);
// ビューポートを設定
D3D11_VIEWPORT vp = {};
vp.TopLeftX = 0;      vp.TopLeftY = 0;
vp.Width = 800.0f; vp.Height = 600.0f;
vp.MinDepth = 0.0f;   vp.MaxDepth = 1.0f;
ctx->RSSetViewports(1, &vp);
//改造後ではレンダリングパイプラインへのセットがオフスクリーンとなっており、
//テクスチャを読み込みして送信している

//※このレンダーターゲットをサーバー側のウィンドウにもセットしておいて、
//更にオフスクリーンに送信することはで//きないか?
//ChatGPTの回答
/*
→サーバー側の「ウィンドウに描く処理」を残しつつ、さらにオフスクリーン用のレンダーターゲットに同じ描画を追加で行うだけであれば、特別なバリアや同期処理は不要です。以下のポイントを押さえれば実現できます。
*/
// ―― ここまで追加 ――
// --- シェーダーコンパイル & 作成 ---
ID3D11VertexShader* vs = nullptr;
ID3D11PixelShader* ps = nullptr;
ID3D11InputLayout* layout = nullptr;

// VS
{
    ID3DBlob* vsBlob = nullptr;
    hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0,
        &vsBlob, nullptr);
    if (FAILED(hr)) {
        OutputDebugString(L"Vertex shader compile failed\n");
        return -1;
    }
    dev->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr, &vs);

    // 入力レイアウト
    D3D11_INPUT_ELEMENT_DESC desc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
          0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    dev->CreateInputLayout(desc, 1,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &layout);
    vsBlob->Release();
}

// PS
{
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr,
        "PSMain", "ps_5_0", 0, 0,
        &psBlob, nullptr);
    if (FAILED(hr)) {
        OutputDebugString(L"Pixel shader compile failed\n");
        return -1;
    }
    dev->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr, &ps);
    psBlob->Release();
}

// --- 頂点バッファ作成 ---
D3D11_BUFFER_DESC bd = {};
bd.Usage = D3D11_USAGE_DEFAULT;
bd.ByteWidth = sizeof(Vertex) * count;
bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
D3D11_SUBRESOURCE_DATA initData = { vertices, 0, 0 };
ID3D11Buffer* vb = nullptr;
dev->CreateBuffer(&bd, &initData, &vb);

// --- メインループ ---
MSG msg = {};
while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    else {

        // ―― オフスクリーンレンダリング & Readback & 送信 ――
        ctx->OMSetRenderTargets(1, &offRtv, nullptr);
//変更点
        // GPU→CPUコピー
        ctx->CopyResource(stagingTex,
            renderTex);

        // Map して一気に送信
        D3D11_MAPPED_SUBRESOURCE m;
        ctx->Map(stagingTex, 0, D3D11_MAP_READ, 0, &m);
        for (int y = 0; y < HEIGHT; ++y) {
            // 各行を TCP 送信 (RGBA8)
            send(client,
                reinterpret_cast<char*>(m.pData) + m.RowPitch * y,
                WIDTH * 4, 0);
        }
        ctx->Unmap(stagingTex, 0);

        // 画面クリア(既存点)
        const FLOAT clearColor[4] = { 0, 0, 0, 1 };
        ctx->ClearRenderTargetView(rtv, clearColor);

        // パイプラインセットアップ
        UINT stride = sizeof(Vertex), offset = 0;
        ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
        ctx->IASetInputLayout(layout);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(vs, nullptr, 0);
        ctx->PSSetShader(ps, nullptr, 0);

        // デバッグログ：描画フェイズ到達
        OutputDebugString(L"Reached Draw Phase\n");

        // 描画
        ctx->Draw(count, 0);

        // 表示
        swap->Present(1, 0);
    }
}

// --- 後片付け ---
vb->Release();
layout->Release();
vs->Release();
ps->Release();
rtv->Release();
swap->Release();
ctx->Release();
dev->Release();
WSACleanup();
delete[] vertices;
offRtv->Release();
stagingTex->Release();
renderTex->Release();
// （シェーダーや VB、dev/ctx、ソケットも適宜 Release/close）
closesocket(client);
WSACleanup();

return 0;
}
