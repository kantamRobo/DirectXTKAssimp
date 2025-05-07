#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <d3d11.h>
#pragma comment(lib, "Ws2_32.lib")

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
// 送りたい三角形の頂点（Z = 0.0f）
struct Vertex {
    float x, y, z;
};
















int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd) {//ここがWinMainになっている。このままでいい





    // グローバルに保持
    static const int WIDTH = 800;
    static const int HEIGHT = 600;




    // Winsock 初期化
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return -1;
    }

    // ソケット作成＆サーバーに接続
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in srvaddr = {};
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &srvaddr.sin_addr);



    if (connect(sock, reinterpret_cast<sockaddr*>(&srvaddr), sizeof(srvaddr)) != 0) {
        std::cerr << "connect() failed\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    // 三角形の頂点データ
    Vertex vertices[3] = {
        {  0.0f,  0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f }
    };
    // 頂点数と頂点データを送信
    int count = 3;
    send(sock, reinterpret_cast<const char*>(&count), sizeof(count), 0);
    send(sock, reinterpret_cast<const char*>(vertices), sizeof(vertices), 0);
    std::cout << "Sent " << count << " vertices to server.\n";



    //ここが微妙に違う(しおり)

    //改造後ではここからD3D11初期化が入る。
      // ―― ここからD3D11初期化 ―― 【追加】  
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = WIDTH;
    scd.BufferDesc.Height = HEIGHT;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = nullptr; // 後でウィンドウ作成時に設定
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    // Win32 ウィンドウ作成【追加】
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"HostWnd";
    RegisterClass(&wc);
    HWND hWnd = CreateWindow(wc.lpszClassName, L"Host Display",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WIDTH, HEIGHT,
        nullptr, nullptr, hInst, nullptr);

    scd.OutputWindow = hWnd; // 【変更】ここで swap chain にウィンドウをセット

    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    IDXGISwapChain* swap = nullptr;
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &swap, &dev, nullptr, &ctx))) {

    //改造後はウィンドウ作成
    //改造後はこのウィンドウにスワップチェーンをセット
        return -1;
    }
    ShowWindow(hWnd, nCmd);

    // RTV とビューポート設定【追加】
    ID3D11Texture2D* backBuf = nullptr;
    swap->GetBuffer(0, IID_PPV_ARGS(&backBuf));
    ID3D11RenderTargetView* rtv = nullptr;
    dev->CreateRenderTargetView(backBuf, nullptr, &rtv);
    // バックバッファを保持したままにする
    ctx->OMSetRenderTargets(1, &rtv, nullptr);

    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)WIDTH;
    vp.Height = (FLOAT)HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    // ―― 画面表示ループ ＋ サーバーからのフレーム受信 ―― 【追加】
    const int frameSize = WIDTH * HEIGHT * 4;
    char* pixelData = new char[frameSize];

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // サーバーからRGBA8フレームを受信
            int received = 0;
            while (received < frameSize) {
                int r = recv(sock,
                    pixelData + received,
                    frameSize - received,
                    0 /* MSG_WAITALL を外す */);
                if (r > 0) {
                    received += r;
                }
                else if (r == 0) {
                    // 接続がクリーンに切断された
                    OutputDebugStringA("recv: connection closed by peer\n");
                    break;
                }
                else { // r == SOCKET_ERROR
                    int err = WSAGetLastError();
                    char buf[128];
                    sprintf_s(buf,
                        "recv failed: WSAGetLastError() = %d\n",
                        err);
                    OutputDebugStringA(buf);
                    break;
                }
            }

            if (received < frameSize) break; // 切断されたらループ抜け
            // バックバッファに直接コピー
            ctx->UpdateSubresource(backBuf, 0, nullptr,
                pixelData, WIDTH * 4, 0);
            ctx->OMSetRenderTargets(1, &rtv, nullptr);
            // 表示
            swap->Present(1, 0);
        }
    }


    // ―― 後片付け ――  
    delete[] pixelData;
    backBuf->Release();
    rtv->Release();
    swap->Release();
    ctx->Release();
    dev->Release();
    closesocket(sock);
    WSACleanup();

    return 0;
}
