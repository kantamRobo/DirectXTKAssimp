#include <winsock2.h>
#include <windows.h>

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

// Win32 ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd) {
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




    SOCKET client = accept(listenSock, nullptr, nullptr);

    // --- 頂点データ受信 ---
    int count = 0;
    Vertex* vertices = new Vertex[count];

    recv(client, reinterpret_cast<char*>(&count), sizeof(count), MSG_WAITALL);
    struct Vertex { float x, y, z; };
    Vertex* verts = new Vertex[count];
    recv(client, reinterpret_cast<char*>(verts), count * sizeof(Vertex), MSG_WAITALL);

    // --- Win32 ウィンドウ作成 ---
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"DX11ServerWnd";
    RegisterClass(&wc);
    HWND hWnd = CreateWindow(wc.lpszClassName, L"Server Renderer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WIDTH, HEIGHT,
        nullptr, nullptr, hInst, nullptr);
    ShowWindow(hWnd, nCmd);

    // --- D3D11 デバイスとスワップチェーン生成 ---
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = WIDTH;
    scd.BufferDesc.Height = HEIGHT;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    IDXGISwapChain* swap = nullptr;
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &swap, &dev, nullptr, &ctx))) {
        MessageBox(hWnd, L"D3D11CreateDeviceAndSwapChain failed", L"Error", MB_OK);
        return -1;
    }

    // --- ウィンドウ用 RTV 作成 ---
    ID3D11Texture2D* backBuf = nullptr;
    swap->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    ID3D11RenderTargetView* windowRtv = nullptr;
    dev->CreateRenderTargetView(backBuf, nullptr, &windowRtv);
    backBuf->Release();

    // --- オフスクリーン用テクスチャ & RTV ---
    D3D11_TEXTURE2D_DESC texDesc;
    backBuf->GetDesc(&texDesc);
    // オフスクリーン用は同じフォーマット・サイズ
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    ID3D11Texture2D* offscreenTex = nullptr;
    dev->CreateTexture2D(&texDesc, nullptr, &offscreenTex);
    ID3D11RenderTargetView* offRtv = nullptr;
    dev->CreateRenderTargetView(offscreenTex, nullptr, &offRtv);

    // 後で読み出し用に backBuf をリリース
    backBuf->Release();
    // --- ステージングテクスチャ（CPU読み出し用） ---
    D3D11_TEXTURE2D_DESC stageDesc = texDesc;
    stageDesc.Usage = D3D11_USAGE_STAGING;
    stageDesc.BindFlags = 0;
    stageDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ID3D11Texture2D* stagingTex = nullptr;
    dev->CreateTexture2D(&stageDesc, nullptr, &stagingTex);

    // --- ビューポート設定 ---
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)WIDTH;
    vp.Height = (FLOAT)HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    // --- シェーダー & レイアウト & VB 作成 ---
    // （既存のVS/PSコンパイルと頂点バッファ生成コードをここに）...
    
        // ―― ここまで追加 ――
        // --- シェーダーコンパイル & 作成 ---
        ID3D11VertexShader * vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11InputLayout* layout = nullptr;

    // VS
    {
        ID3DBlob* vsBlob = nullptr;
       auto  hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr,
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
        auto hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr,
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

    // --- メイン描画 & 送信ループ ---
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // RTV をウィンドウ & オフスクリーン両方にバインド
            ID3D11RenderTargetView* rtvs[2] = { windowRtv, offRtv };
            ctx->OMSetRenderTargets(2, rtvs, nullptr);
            // クリア
            const FLOAT clearColor[4] = { 0,0,0,1 };
            ctx->ClearRenderTargetView(windowRtv, clearColor);
            ctx->ClearRenderTargetView(offRtv, clearColor);

            // パイプラインセットアップ & Draw
            // （IASet... / VSSetShader / PSSetShader / ctx->Draw(count,0)）

            // ウィンドウ表示
            swap->Present(1, 0);

            // オフスクリーンをステージングへコピー & Readback
            ctx->CopyResource(stagingTex, offscreenTex);
            D3D11_MAPPED_SUBRESOURCE m;
            ctx->Map(stagingTex, 0, D3D11_MAP_READ, 0, &m);
            for (int y = 0; y < HEIGHT; ++y) {
                send(client,
                    reinterpret_cast<char*>(m.pData) + (size_t)m.RowPitch * y,
                    WIDTH * 4, 0);
            }
            ctx->Unmap(stagingTex, 0);
        }
    }

    // --- 後片付け ---
    offRtv->Release(); stagingTex->Release(); offscreenTex->Release();
    windowRtv->Release(); swap->Release(); ctx->Release(); dev->Release();
    closesocket(client); WSACleanup(); delete[] verts;
    return 0;
}
