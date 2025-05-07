#include <winsock2.h>
#include <windows.h>

#include <ws2tcpip.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
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
    if (recv(client, reinterpret_cast<char*>(&count), sizeof(count), MSG_WAITALL) != sizeof(count)) {
        OutputDebugStringA("Failed to receive vertex count\n");
        closesocket(client);
        closesocket(listenSock);
        WSACleanup();
        return -1;
    }
    Vertex* verts = new Vertex[count];
    int bytesToReceive = count * sizeof(Vertex);
    int totalReceived = 0;
    while (totalReceived < bytesToReceive) {
        int r = recv(client,
            reinterpret_cast<char*>(verts) + totalReceived,
            bytesToReceive - totalReceived,
            0);
        if (r > 0) {
            totalReceived += r;
        }
        else if (r == 0) {
            OutputDebugStringA("recv: connection closed by peer while receiving vertices\n");
            break;
        }
        else { // SOCKET_ERROR
            int err = WSAGetLastError();
            char buf[128];
            sprintf_s(buf, "recv failed: WSAGetLastError() = %d\n", err);
            OutputDebugStringA(buf);
            break;
        }
    }
    if (totalReceived != bytesToReceive) {
        delete[] verts;
        closesocket(client);
        closesocket(listenSock);
        WSACleanup();
        return -1;
    }

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
        delete[] verts;
        closesocket(client);
        closesocket(listenSock);
        WSACleanup();
        return -1;
    }

    // --- レンダーターゲットビュー作成 ---
    ID3D11Texture2D* backBuf = nullptr;
    ID3D11RenderTargetView* rtvWin = nullptr;
    swap->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    dev->CreateRenderTargetView(backBuf, nullptr, &rtvWin);

    // --- オフスクリーン用テクスチャ & RTV ---
    D3D11_TEXTURE2D_DESC texDesc;
    backBuf->GetDesc(&texDesc);
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    ID3D11Texture2D* offTex = nullptr;
    ID3D11RenderTargetView* rtvOff = nullptr;
    dev->CreateTexture2D(&texDesc, nullptr, &offTex);
    dev->CreateRenderTargetView(offTex, nullptr, &rtvOff);
    backBuf->Release();

    // --- ステージングテクスチャ作成 ---
    D3D11_TEXTURE2D_DESC stageDesc = texDesc;
    stageDesc.Usage = D3D11_USAGE_STAGING;
    stageDesc.BindFlags = 0;
    stageDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ID3D11Texture2D* stageTex = nullptr;
    dev->CreateTexture2D(&stageDesc, nullptr, &stageTex);

    // --- ビューポート設定 ---
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)WIDTH;
    vp.Height = (FLOAT)HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    // --- シェーダーコンパイル & 入力レイアウト & VB生成 ---
    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11InputLayout* layout = nullptr;
    ID3D11Buffer* vb = nullptr;

    // Vertex Shader
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"VS.hlsl", nullptr, nullptr,
        "VSMain", "vs_5_0",
        0, 0, &vsBlob, nullptr);
    if (FAILED(hr)) {
        OutputDebugStringA("Vertex shader compile failed\n");
        return -1;
    }
    dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs);

    // Input Layout
    D3D11_INPUT_ELEMENT_DESC ieDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
          offsetof(Vertex, x), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    dev->CreateInputLayout(ieDesc, _countof(ieDesc),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        &layout);
    vsBlob->Release();

    // Pixel Shader
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompileFromFile(L"PS.hlsl", nullptr, nullptr,
        "PSMain", "ps_5_0",
        0, 0, &psBlob, nullptr);
    if (FAILED(hr)) {
        OutputDebugStringA("Pixel shader compile failed\n");
        return -1;
    }
    dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps);
    psBlob->Release();

    // Vertex Buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * count;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA initData = { verts, 0, 0 };
    dev->CreateBuffer(&bd, &initData, &vb);

    // --- メイン描画 & 送信ループ ---
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            ID3D11RenderTargetView* rtvs[2] = { rtvWin, rtvOff };
            ctx->OMSetRenderTargets(2, rtvs, nullptr);
            const FLOAT clearC[4] = { 0,0,0,1 };
            ctx->ClearRenderTargetView(rtvWin, clearC);
            ctx->ClearRenderTargetView(rtvOff, clearC);
            // 頂点構造体あたりのバイト数
            UINT stride = sizeof(Vertex);
            // バッファの先頭オフセット
            UINT offset = 0;

            // vb は作成済みの ID3D11Buffer*
            ctx->IASetVertexBuffers(
                0,       // スロット番号
                1,       // バッファ数
                &vb,     // バッファ配列の先頭アドレス
                &stride, // 1 要素あたりのバイト数
                &offset  // バッファ内オフセット
            );

            ctx->IASetInputLayout(layout);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            ctx->VSSetShader(vs, nullptr, 0);
            ctx->PSSetShader(ps, nullptr, 0);
            ctx->Draw(count, 0);
            swap->Present(1, 0);

            // Off-screen -> Reading -> Send
            ctx->CopyResource(stageTex, offTex);
            D3D11_MAPPED_SUBRESOURCE mres;
            ctx->Map(stageTex, 0, D3D11_MAP_READ, 0, &mres);
            for (int y = 0; y < HEIGHT; ++y) {
                send(client,
                    reinterpret_cast<char*>(mres.pData) + mres.RowPitch * y,
                    WIDTH * 4, 0);
            }
            ctx->Unmap(stageTex, 0);
        }
    }

    // --- 後片付け ---
    vb->Release(); layout->Release(); vs->Release(); ps->Release();
    rtvOff->Release(); stageTex->Release(); offTex->Release();
    rtvWin->Release(); swap->Release(); ctx->Release(); dev->Release();
    shutdown(client, SD_SEND);
    closesocket(client);
    closesocket(listenSock);
    WSACleanup();
    delete[] verts;
    return 0;
}
