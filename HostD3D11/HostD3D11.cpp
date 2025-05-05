#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

// 送りたい三角形の頂点（Z = 0.0f）
struct Vertex {
    float x, y, z;
};

int main() {
    // 三角形の頂点データ
    Vertex vertices[3] = {
        {  0.0f,  0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f }
    };

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

    // 頂点数と頂点データを送信
    int count = 3;
    send(sock, reinterpret_cast<const char*>(&count), sizeof(count), 0);
    send(sock, reinterpret_cast<const char*>(vertices), sizeof(vertices), 0);
    std::cout << "Sent " << count << " vertices to server.\n";

    // 後片付け
    closesocket(sock);
    WSACleanup();
    return 0;
}
