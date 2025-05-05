#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

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
            // 画面クリア
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
    return 0;
}
