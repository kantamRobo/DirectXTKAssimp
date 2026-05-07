#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;

    m_deviceResources->SetWindow(window, width, height);
    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
}

void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

void Game::Update(DX::StepTimer const& timer)
{
    const float dt = static_cast<float>(timer.GetElapsedSeconds());
    m_elapsedTotal += dt;

    const bool moved = UpdateCamera(dt);

    XMVECTOR forward = XMVectorSet(
        cosf(m_cameraPitch) * sinf(m_cameraYaw),
        sinf(m_cameraPitch),
        cosf(m_cameraPitch) * cosf(m_cameraYaw),
        0.0f);
    forward = XMVector3Normalize(forward);

    const XMVECTOR worldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(worldUp, forward));
    XMVECTOR up = XMVector3Normalize(XMVector3Cross(forward, right));

    XMFLOAT3 forwardF;
    XMFLOAT3 rightF;
    XMFLOAT3 upF;
    XMStoreFloat3(&forwardF, forward);
    XMStoreFloat3(&rightF, right);
    XMStoreFloat3(&upF, up);

    m_pathTracer.SetCamera(m_cameraPosition, forwardF, rightF, upF, XM_PIDIV4, moved);
    UpdateWindowTitle(m_elapsedTotal);
}

void Game::Render()
{
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"PathTracing");
    m_pathTracer.Render(m_deviceResources.get(), m_elapsedTotal);
    m_deviceResources->PIXEndEvent();

    m_deviceResources->Present();
}

void Game::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    const auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);
}

void Game::OnActivated() {}
void Game::OnDeactivated() {}
void Game::OnSuspending() {}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    width = 1280;
    height = 720;
}

void Game::CreateDeviceDependentResources()
{
    auto* device = m_deviceResources->GetD3DDevice();
    const auto output = m_deviceResources->GetOutputSize();
    const uint32_t width = static_cast<uint32_t>(std::max<LONG>(1, output.right - output.left));
    const uint32_t height = static_cast<uint32_t>(std::max<LONG>(1, output.bottom - output.top));
    m_pathTracer.Initialize(m_deviceResources.get(), width, height);
    (void)device;
}

void Game::CreateWindowSizeDependentResources()
{
    const auto output = m_deviceResources->GetOutputSize();
    const uint32_t width = static_cast<uint32_t>(std::max<LONG>(1, output.right - output.left));
    const uint32_t height = static_cast<uint32_t>(std::max<LONG>(1, output.bottom - output.top));
    m_pathTracer.OnResize(m_deviceResources.get(), width, height);
}

void Game::OnDeviceLost()
{
    m_pathTracer.OnDeviceLost();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

bool Game::UpdateCamera(float dt)
{
    bool changed = false;

    auto isDown = [](int key) -> bool
    {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    };

    const float move = m_cameraMoveSpeed * dt;
    const float rotate = m_cameraRotateSpeed * dt;

    XMVECTOR forward = XMVectorSet(cosf(m_cameraPitch) * sinf(m_cameraYaw), sinf(m_cameraPitch), cosf(m_cameraPitch) * cosf(m_cameraYaw), 0.0f);
    forward = XMVector3Normalize(forward);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), forward));

    XMVECTOR position = XMLoadFloat3(&m_cameraPosition);

    if (isDown('W')) { position = XMVectorAdd(position, XMVectorScale(forward, move)); changed = true; }
    if (isDown('S')) { position = XMVectorSubtract(position, XMVectorScale(forward, move)); changed = true; }
    if (isDown('A')) { position = XMVectorSubtract(position, XMVectorScale(right, move)); changed = true; }
    if (isDown('D')) { position = XMVectorAdd(position, XMVectorScale(right, move)); changed = true; }
    if (isDown('Q')) { position = XMVectorAdd(position, XMVectorSet(0.f, move, 0.f, 0.f)); changed = true; }
    if (isDown('E')) { position = XMVectorSubtract(position, XMVectorSet(0.f, move, 0.f, 0.f)); changed = true; }

    if (isDown(VK_LEFT)) { m_cameraYaw -= rotate; changed = true; }
    if (isDown(VK_RIGHT)) { m_cameraYaw += rotate; changed = true; }
    if (isDown(VK_UP)) { m_cameraPitch += rotate; changed = true; }
    if (isDown(VK_DOWN)) { m_cameraPitch -= rotate; changed = true; }

    m_cameraPitch = std::clamp(m_cameraPitch, -1.4f, 1.4f);

    XMStoreFloat3(&m_cameraPosition, position);
    return changed;
}

void Game::UpdateWindowTitle(float elapsedSeconds)
{
    m_titleUpdateAccumulator += static_cast<float>(m_timer.GetElapsedSeconds());
    if (m_titleUpdateAccumulator < 0.25f)
    {
        return;
    }
    m_titleUpdateAccumulator = 0.0f;

    wchar_t title[256] = {};
    swprintf_s(
        title,
        L"DirectXTKPathTracer | spp=%u | bounces=%u | time=%.2fs",
        m_pathTracer.GetSampleCount(),
        m_pathTracer.GetMaxBounces(),
        elapsedSeconds);
    SetWindowTextW(m_window, title);
}
