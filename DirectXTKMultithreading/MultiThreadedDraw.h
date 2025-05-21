// MultiThreadedDraw.h
#pragma once

#include <wrl/client.h>
#include <d3d11_4.h>
#include <vector>
#include <thread>
#include <memory>

#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include "DeviceResources.h"

class MultiThreadedDraw
{
public:
    explicit MultiThreadedDraw(
        std::shared_ptr<DX::DeviceResources> const& deviceResources,
        size_t workerCount = std::thread::hardware_concurrency());
    ~MultiThreadedDraw();

    void Update(float dt);
    void Render();

private:
    void RecordWorker(size_t i);

    struct Worker
    {
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>  ctx;   // Deferred Context
        Microsoft::WRL::ComPtr<ID3D11CommandList>     cl;    // �R�}���h���X�g
        DirectX::SimpleMath::Matrix                  world; // ���[���h�s��
        std::unique_ptr<DirectX::GeometricPrimitive> prim;  // �v���~�e�B�u
    };

    std::shared_ptr<DX::DeviceResources> m_deviceResources;
    std::vector<Worker>                  m_workers;

    // �ǉ��F�J�����s���ێ����Ă���
    DirectX::SimpleMath::Matrix         m_view;
    DirectX::SimpleMath::Matrix         m_proj;

};
