// Server.cpp
#include <winsock2.h>
#include <iostream>
#include "DrawCommand.h"
#pragma comment(lib, "ws2_32.lib")

int main()
{
    // 1) WSA 初期化
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // 2) ソケット作成・バインド・リッスン
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in srvAddr = {};
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(12345);
    srvAddr.sin_addr.s_addr = INADDR_ANY;
    bind(listenSock, (sockaddr*)&srvAddr, sizeof(srvAddr));
    listen(listenSock, 1);
    std::cout << "Listening on port 12345...\n";

    // 3) クライアント受け入れ
    SOCKET clientSock = accept(listenSock, nullptr, nullptr);
    std::cout << "Client connected\n";

    // 4) 三角形データを準備
    DrawCommand cmd = {};
    // 頂点座標 (例：2D 画面中央に黄色い三角形)
    cmd.vertices[0][0] = 0.0f; cmd.vertices[0][1] = 0.5f; cmd.vertices[0][2] = 0.5f;
    cmd.vertices[1][0] = 0.5f; cmd.vertices[1][1] = -0.5f; cmd.vertices[1][2] = 0.5f;
    cmd.vertices[2][0] = -0.5f; cmd.vertices[2][1] = -0.5f; cmd.vertices[2][2] = 0.5f;
    // 色 (黄色)
    cmd.color[0] = 1.0f; cmd.color[1] = 1.0f; cmd.color[2] = 0.0f;

    // 5) 送信
    int total = sizeof(cmd);
    char* ptr = reinterpret_cast<char*>(&cmd);
    while (total > 0) {
        int sent = send(clientSock, ptr, total, 0);
        if (sent == SOCKET_ERROR) break;
        ptr += sent;
        total -= sent;
    }
    std::cout << "DrawCommand sent\n";

    // 6) クリーンアップ
    closesocket(clientSock);
    closesocket(listenSock);
    WSACleanup();

    return 0;
}
