//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <array>
extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    using std::max; using std::min;

    double rawSeconds = timer.GetElapsedSeconds();
    // 1) フレーム経過時間（0 以上、最大1時間にクランプ）
    float safeFrame = std::isfinite(rawSeconds) ? static_cast<float>(rawSeconds) : 0.0f;
    const float kMaxSeconds = 3600.0f; // 1 hour cap to avoid runaway values
    safeFrame = max(0.0f, min(safeFrame, kMaxSeconds));

    // 2) 累積時間から得る回転角（[0, 2π) にバウンド）
    static float s_totalTime = 0.0f;
    s_totalTime += safeFrame;
    if (!std::isfinite(s_totalTime) || s_totalTime > 1e8f) s_totalTime = 0.0f; // 安全リセット
    float safeAngle = fmodf(s_totalTime, 2.0f * XM_PI);
    if (!std::isfinite(safeAngle)) safeAngle = 0.0f;

    // 追加: 経過時間（ボーンアニメーション用）
    float elapsedTime = s_totalTime;

    // 3) 非ゼロかつ有限な delta（除算や正規化などで使う安全値）
    const float kMinDelta = 1e-6f;
    float safeDelta = max(safeFrame, kMinDelta);

    // シーン行列（World, View, Projection）とボーン行列を計算して SkinnedMesh に送る

    if (!m_skinnedMesh || !m_deviceResources)
        return;

    // 画面アスペクトを取得
    auto r = m_deviceResources->GetOutputSize();
    float width = static_cast<float>(r.right - r.left);
    float height = static_cast<float>(r.bottom - r.top);
    float aspect = (height != 0.0f) ? (width / height) : 1.0f;

    // World: Y 回転と少しのスケール
    float angle = safeFrame; // 秒に応じて回転
    XMMATRIX worldMat = XMMatrixRotationY(angle) * XMMatrixScaling(1.0f, 1.0f, 1.0f);
    XMFLOAT4X4 worldXF;
    XMStoreFloat4x4(&worldXF, XMMatrixTranspose(worldMat)); // HLSL 側が列優先と仮定して転置して渡す（既存コードが直接コピーしているため）
    // View: カメラ位置
    XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -3.0f, 0.0f);
    XMVECTOR at  = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR up  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX viewMat = XMMatrixLookAtLH(eye, at, up);
    XMFLOAT4X4 viewXF;
    XMStoreFloat4x4(&viewXF, XMMatrixTranspose(viewMat));

    // Projection: 標準的な透視投影
    XMMATRIX projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);
    XMFLOAT4X4 projXF;
    XMStoreFloat4x4(&projXF, XMMatrixTranspose(projMat));

    // UpdateSceneMatrices に渡す（SkinnedMesh 側で定数バッファへ転送される）
    HRESULT hrScene = m_skinnedMesh->UpdateSceneMatrices(m_deviceResources.get(), worldXF, viewXF, projXF);
    (void)hrScene; // 必要ならログ処理を追加

    // ボーン行列データの準備（最大64）
    {
        const size_t boneCount = 64;
        std::vector<XMFLOAT4X4> boneTransforms;
        boneTransforms.resize(boneCount);

        // 元の頂点座標（7頂点 + 中央）
        std::array<XMFLOAT3, 8> orig = {
            XMFLOAT3(0.0f,   0.6f,  0.0f),   // 0:上
            XMFLOAT3(0.4f,   0.3f,  0.0f),   // 1:右上
            XMFLOAT3(0.6f,  -0.2f,  0.0f),   // 2:右
            XMFLOAT3(0.3f,  -0.5f,  0.0f),   // 3:右下
            XMFLOAT3(-0.3f, -0.5f,  0.0f),   // 4:左下
            XMFLOAT3(-0.6f, -0.2f,  0.0f),   // 5:左
            XMFLOAT3(-0.4f,  0.3f,  0.0f),   // 6:左上
            XMFLOAT3(0.0f,   0.0f,  0.0f)    // 7:中央
        };

        // 目標形状：テキトーな変形（渦巻き的な動き）
        std::array<XMFLOAT3, 8> target = {
            XMFLOAT3(0.0f,   0.7f,  0.0f),   // 0:上へ伸長
            XMFLOAT3(0.5f,   0.5f,  0.0f),   // 1:右上へ移動
            XMFLOAT3(0.7f,   0.0f,  0.0f),   // 2:右へ伸長
            XMFLOAT3(0.4f,  -0.6f,  0.0f),   // 3:右下へ
            XMFLOAT3(-0.4f, -0.6f,  0.0f),   // 4:左下へ
            XMFLOAT3(-0.7f,  0.0f,  0.0f),   // 5:左へ伸長
            XMFLOAT3(-0.5f,  0.5f,  0.0f),   // 6:左上へ移動
            XMFLOAT3(0.1f,  -0.1f,  0.0f)    // 7:中央がずれる
        };

        // ボーン0..7 を各頂点の変換でマップ
        for (size_t i = 0; i < 8; ++i)
        {
            // 元頂点位置
            XMFLOAT3 pivot = orig[i];

            // 目標への平行移動
            XMFLOAT3 d;
            d.x = target[i].x - orig[i].x;
            d.y = target[i].y - orig[i].y;
            d.z = target[i].z - orig[i].z;

            // 時間応じた回転（各ボーンで異なる速度）
            float angle = elapsedTime * (0.3f + 0.15f * static_cast<float>(i));

            // ピボット周りの回転 + 移動
            XMMATRIX T_negPivot = XMMatrixTranslation(-pivot.x, -pivot.y, -pivot.z);
            XMMATRIX R = XMMatrixRotationZ(angle);
            XMMATRIX T_pivot = XMMatrixTranslation(pivot.x, pivot.y, pivot.z);
            XMMATRIX T_move = XMMatrixTranslation(d.x, d.y, d.z);

            XMMATRIX m = T_move * (T_pivot * R * T_negPivot);

            XMFLOAT4X4 xf;
            XMStoreFloat4x4(&xf, XMMatrixTranspose(m));
            boneTransforms[i] = xf;
        }

        // 残りのボーンは単位行列
        for (size_t i = 8; i < boneCount; ++i)
        {
            XMMATRIX id = XMMatrixIdentity();
            XMFLOAT4X4 xf;
            XMStoreFloat4x4(&xf, XMMatrixTranspose(id));
            boneTransforms[i] = xf;
        }

        // GPU にアップロード
        HRESULT hrBones = m_skinnedMesh->UpdateBoneTransforms(m_deviceResources.get(), boneTransforms);
        (void)hrBones;
    }
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // 描画呼び出し: SkinnedMesh が初期化されていれば描画する
    if (m_skinnedMesh)
    {
        m_skinnedMesh->Draw(m_deviceResources.get());
    }

    m_deviceResources->PIXEndEvent();

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    const auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    const auto r = m_deviceResources->GetOutputSize();
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

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // SkinnedMesh の生成と初期化
    m_skinnedMesh = std::make_unique<SkinnedMesh>();

    // 出力サイズを取得してバッファ作成に渡す
    auto r = m_deviceResources->GetOutputSize();
    int w = r.right - r.left;
    int h = r.bottom - r.top;

    // 順に初期化: バッファ、シェーダー、パイプライン
    if (FAILED(m_skinnedMesh->CreateBuffers(m_deviceResources.get(), w, h)))
    {
        // エラー処理（必要に応じてログや例外）
    }
    if (FAILED(m_skinnedMesh->CreateShaders(m_deviceResources.get())))
    {
        // エラー処理
    }
    m_skinnedMesh->CreatePipelineStates(m_deviceResources.get(), m_skinnedMesh.get());
    {

        // TODO: Initialize other device dependent objects here (independent of window size).
        device;
    }
}
// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // 必要であればここで SkinnedMesh のウィンドウサイズ依存処理を追加
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    // SkinnedMesh のリソースを解放（スマートポインタなので reset）
    m_skinnedMesh.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
