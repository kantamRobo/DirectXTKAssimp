//
// Game.h
//

#pragma once
#include "pch.h"
#include <memory>
#include "StepTimer.h"
#include <GraphicsMemory.h>
// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

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
    void GetDefaultSize( int& width, int& height ) const noexcept;
     int m_width;
     int m_height;
    void SetViewPort(int width, int height)
    {
        m_width = width;
        m_height = height;
    }
private:
    std::unique_ptr<education::Model> model;
    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsmemory;
    // Rendering loop timer.
    DX::StepTimer                           m_timer;
};
