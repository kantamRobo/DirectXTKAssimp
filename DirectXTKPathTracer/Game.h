#pragma once

#include "ComputeShader.h"
#include "DeviceResources.h"
#include "StepTimer.h"

class Game final : public DX::IDeviceNotify
{
public:
    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator=(Game&&) = default;

    Game(Game const&) = delete;
    Game& operator=(Game const&) = delete;

    void Initialize(HWND window, int width, int height);
    void Tick();

    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    void GetDefaultSize(int& width, int& height) const noexcept;

private:
    void Update(DX::StepTimer const& timer);
    void Render();
    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    bool UpdateCamera(float dt);
    void UpdateWindowTitle(float elapsedSeconds);

    std::unique_ptr<DX::DeviceResources> m_deviceResources;
    DX::StepTimer m_timer;

    ComputeShader m_pathTracer;

    HWND m_window = nullptr;
    float m_elapsedTotal = 0.0f;

    DirectX::XMFLOAT3 m_cameraPosition = { 0.0f, 1.2f, -6.0f };
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = 0.0f;
    float m_cameraMoveSpeed = 3.0f;
    float m_cameraRotateSpeed = 1.5f;

    float m_titleUpdateAccumulator = 0.0f;
};
