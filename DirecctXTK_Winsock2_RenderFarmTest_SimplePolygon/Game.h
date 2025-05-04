// Game.h
#pragma once

#include "pch.h"
#include "DeviceResources.h"
#include "StepTimer.h"
#include "DrawCommand.h"

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:
    Game() noexcept(false);
    ~Game() = default;
    Game(Game&&) = default;
    Game& operator=(Game&&) = default;
    Game(Game const&) = delete;
    Game& operator=(Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:
    void Update(DX::StepTimer const& timer);
    void Render();
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // --- DirectXTK 用メンバ ---
    using VertexType = DirectX::VertexPositionColor;
    std::unique_ptr<DirectX::CommonStates>               m_states;
    std::unique_ptr<DirectX::BasicEffect>                m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>            m_inputLayout;

    // --- ネットワーク用メンバ ---
    SOCKET        m_socket = INVALID_SOCKET;
    DrawCommand   m_cmd{};      // 受信した描画命令

    // Device resources.
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                      m_timer;

    // ネットワーク初期化
    void InitNetwork();
};
