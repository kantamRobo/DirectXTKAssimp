// Game.cpp
#include "pch.h"
#include "Game.h"
#include <iostream>
#include <SimpleMath.h>
extern void ExitGame() noexcept;

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);
    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();
    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::Tick()
{
    m_timer.Tick([&]() { Update(m_timer); });
    Render();
}

void Game::Update(DX::StepTimer const& timer)
{
    // ゲームロジックは特になし
    (void)timer;
}

void Game::Render()
{
    if (m_timer.GetFrameCount() == 0) return;

    Clear();
    m_deviceResources->PIXBeginEvent(L"Render");

    auto context = m_deviceResources->GetD3DDeviceContext();

    // 受信したコマンドで三角形描画
 
    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());


    VertexPositionColor v1(Vector3(0.f, 0.5f, 0.5f), Colors::Yellow);
    VertexPositionColor v2(Vector3(0.5f, -0.5f, 0.5f), Colors::Yellow);
    VertexPositionColor v3(Vector3(-0.5f, -0.5f, 0.5f), Colors::Yellow);

    m_batch->Begin();
    m_batch->DrawTriangle(v1, v2, v3);
    m_batch->End();

    m_deviceResources->PIXEndEvent();

    m_deviceResources->Present();
}

void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    auto context = m_deviceResources->GetD3DDeviceContext();
    auto rtv = m_deviceResources->GetRenderTargetView();
    auto dsv = m_deviceResources->GetDepthStencilView();
    auto vp = m_deviceResources->GetScreenViewport();

    context->ClearRenderTargetView(rtv, Colors::CornflowerBlue);
    context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &rtv, dsv);
    context->RSSetViewports(1, &vp);

    

    m_deviceResources->PIXEndEvent();
}

void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    // DirectXTK 初期化
    m_states = std::make_unique<DirectX::CommonStates>(device);
    m_effect = std::make_unique< DirectX::BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);

    DX::ThrowIfFailed(
        DirectX::CreateInputLayoutFromEffect<VertexType>(device, m_effect.get(),
            m_inputLayout.ReleaseAndGetAddressOf())
    );

    InitNetwork();  // ← ここでサーバーへ接続、コマンドを受信

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_batch = std::make_unique<DirectX::PrimitiveBatch<VertexType>>(context);
}

void Game::CreateWindowSizeDependentResources()
{
    // ここでは特に何もしない
}

void Game::OnDeviceLost()
{
    // リソース解放
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();

    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        WSACleanup();
        m_socket = INVALID_SOCKET;
    }
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::OnActivated() {}
void Game::OnDeactivated() {}
void Game::OnSuspending() {}
void Game::OnResuming() {}
void Game::OnWindowMoved() {}
void Game::OnDisplayChange() {}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (m_deviceResources->WindowSizeChanged(width, height))
    {
        CreateWindowSizeDependentResources();
    }
}

void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 800;
    height = 600;
}

// — ネットワーク初期化実装 —
void Game::InitNetwork()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return;
    }
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in srv = {};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(12345);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");  // サーバー IP

    if (connect(m_socket, (sockaddr*)&srv, sizeof(srv)) == SOCKET_ERROR)
    {
        std::cerr << "connect failed\n";
        return;
    }

    // DrawCommand の受信
    char* ptr = reinterpret_cast<char*>(&m_cmd);
    int   total = sizeof(m_cmd);
    while (total > 0)
    {
        int rec = recv(m_socket, ptr, total, 0);
        if (rec <= 0) { std::cerr << "recv error\n"; break; }
        ptr += rec;
        total -= rec;
    }
    std::cout << "DrawCommand received\n";
}
