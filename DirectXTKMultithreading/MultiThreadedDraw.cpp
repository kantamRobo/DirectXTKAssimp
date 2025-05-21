// MultiThreadedDraw.cpp
#include "MultiThreadedDraw.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

MultiThreadedDraw::MultiThreadedDraw(
    std::shared_ptr<DX::DeviceResources> const& dr,
    size_t workerCount)
    : m_deviceResources(dr)
    , m_workers(workerCount)
{
    auto device = dr->GetD3DDevice();

    // １) スレッドセーフ化
    ComPtr<ID3D11Multithread> mt;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&mt))))
        mt->SetMultithreadProtected(TRUE);

    // ２) カメラ (View / Proj) を設定
    {
        auto vp = m_deviceResources->GetScreenViewport();
        float aspect = vp.Width / vp.Height;
        // カメラは (0,2,-10) → 原点 を見る
        m_view = SimpleMath::Matrix::CreateLookAt(
            { 0, 2, -10 }, { 0, 0, 0 }, { 0, 1, 0 });
        m_proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(
            XM_PIDIV4, 2, 0.1f, 100.0f);
    }

    // ３) 各 Worker の初期化
    for (size_t i = 0; i < workerCount; ++i)
    {
        auto& w = m_workers[i];

        // Deferred Context
        device->CreateDeferredContext1(0, w.ctx.GetAddressOf());

        // プリミティブ生成 (Deferred Context を渡す)
        w.prim = GeometricPrimitive::CreateCube(w.ctx.Get(), 1.0f);

        // world は Update() で設定するのでここでは単位行列
        w.world = SimpleMath::Matrix::Identity;

        // コマンドリストを「空」で初期化
        w.ctx->ClearState();
        w.ctx->FinishCommandList(FALSE, w.cl.GetAddressOf());
    }
}

void MultiThreadedDraw::Update(float dt)
{
    float angle = dt * XM_PIDIV4;
    size_t N = m_workers.size();
    float spacing = 3.0f;  // キューブ同士の間隔

    for (size_t i = 0; i < N; ++i)
    {
        // 中心を原点に並べるオフセット
        float offset = (static_cast<float>(i) - (N - 1) * 0.5f) * spacing;

        // 回転 ＋ 平行移動
        m_workers[i].world =
            SimpleMath::Matrix::CreateRotationY(angle + static_cast<float>(i)) *
            SimpleMath::Matrix::CreateTranslation(offset, 0.0f, 0.0f);
    }
}

void MultiThreadedDraw::Render()
{
    auto immediate = m_deviceResources->GetD3DDeviceContext();

    // 即時コンテキスト側でクリア（Game.cpp 側でも行って OK）
    immediate->ClearRenderTargetView(
        m_deviceResources->GetRenderTargetView(),
        DirectX::Colors::CornflowerBlue);
    immediate->ClearDepthStencilView(
        m_deviceResources->GetDepthStencilView(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // ４) 並列でコマンド記録
    std::vector<std::thread> threads;
    for (size_t i = 0; i < m_workers.size(); ++i)
        threads.emplace_back(&MultiThreadedDraw::RecordWorker, this, i);
    for (auto& t : threads) t.join();

    // ５) 一括実行
    for (auto& w : m_workers)
    {
        immediate->ExecuteCommandList(w.cl.Get(), FALSE);
        w.cl.Reset();
    }
}

void MultiThreadedDraw::RecordWorker(size_t i)
{
    auto& w = m_workers[i];
    auto  ctx = w.ctx.Get();

    // ステート初期化
    ctx->ClearState();

    // Deferred Context は即時側の OM/VP を継承しないので再設定
    ID3D11RenderTargetView* rtv = m_deviceResources->GetRenderTargetView();
    ID3D11DepthStencilView* dsv = m_deviceResources->GetDepthStencilView();
    ctx->OMSetRenderTargets(1, &rtv, dsv);

    auto vp = m_deviceResources->GetScreenViewport();
    ctx->RSSetViewports(1, &vp);

    // 描画：overload 版 Draw(context, world, view, proj, color)
    w.prim->Draw(w.world, m_view, m_proj, Colors::Gray);

    // コマンドリスト化
    ctx->FinishCommandList(FALSE, w.cl.GetAddressOf());
}
MultiThreadedDraw::~MultiThreadedDraw()
{
    // (A) 即時コンテキストを取得して Flush
    auto immediate = m_deviceResources->GetD3DDeviceContext();
    immediate->Flush();

    // (B) ワーカーごとに State Clear + COM Reset
    for (auto& w : m_workers)
    {
        if (w.ctx) w.ctx->ClearState();
        w.cl.Reset();
        w.prim.reset();
        w.ctx.Reset();
    }
    m_workers.clear();
#if defined(_DEBUG)
    // ReportLiveDeviceObjects で詳細レポート
    Microsoft::WRL::ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(m_deviceResources->GetD3DDevice()->QueryInterface(IID_PPV_ARGS(&d3dDebug))))
    {
        d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
#endif

}
