#pragma once
#include <wrl.h>
#include <d3dcompiler.h>
#include <DeviceResources.h>
#include <d3d11.h>
// �萔�o�b�t�@�p�\����
struct CB_TESS
{
    float Inner;    // �����e�b�Z���[�V�����W��
    float Outer;    // �O���e�b�Z���[�V�����W��
    float pad[2];
};
class D3D11HelloTessellelation
{
public:
    // �V�F�[�_�[�R���p�C��
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, hsBlob, dsBlob, psBlob, errBlob;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>g_cbTessFactor;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    Microsoft::WRL::ComPtr<ID3D11HullShader> hs;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> ds;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    void Render(DX::DeviceResources* DR)
    {

        auto context = DR->GetD3DDeviceContext();
        // �V�F�[�_�[�^���̓��C�A�E�g�ݒ�
        context->IASetInputLayout(inputLayout.Get());
        // �p�b�`���X�g�łR���_�P��
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

        context->VSSetShader(vs.Get(), nullptr, 0);
        context->HSSetShader(hs.Get(), nullptr, 0);
        context->DSSetShader(ds.Get(), nullptr, 0);
        context->PSSetShader(ps.Get(), nullptr, 0);

        // �萔�o�b�t�@�Ƀe�b�Z�W������������
        CB_TESS cb;
        cb.Inner = 5.0f;   // �����e�b�Z
        cb.Outer = 5.0f;   // �O���e�b�Z
        context->UpdateSubresource(g_cbTessFactor.Get(), 0, nullptr, &cb, 0, 0);
        // HS �X�e�[�W�Ƀo�C���h
        context->HSSetConstantBuffers(0, 1, g_cbTessFactor.GetAddressOf());

        // ���_�o�b�t�@�ݒ�
        UINT stride = sizeof(float) * 3;
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
        context->RSSetState(m_rasterizerState.Get());

        // �h���[
        context->Draw(3, 0);

        // ���Z�b�g�i���t���[���̂��߁j
        context->HSSetShader(nullptr, nullptr, 0);
        context->DSSetShader(nullptr, nullptr, 0);

    }
    HRESULT Init(DX::DeviceResources* DR) {
        auto device = DR->GetD3DDevice();
        
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG;
#endif
        // ����t�@�C�����Ɋe�X�e�[�W����`����Ă���̂ŁA�G���g���Ő؂�ւ�
        if (FAILED(D3DCompileFromFile(L"TesselletionVS.hlsl", nullptr, nullptr,
            "VSMain", "vs_5_0", flags, 0, &vsBlob, &errBlob))) return E_FAIL;
        if (FAILED(D3DCompileFromFile(L"TesselletionHS.hlsl", nullptr, nullptr,
            "HSMain", "hs_5_0", flags, 0, &hsBlob, &errBlob))) return E_FAIL;
        if (FAILED(D3DCompileFromFile(L"TesselletionDS.hlsl", nullptr, nullptr,
            "DSMain", "ds_5_0", flags, 0, &dsBlob, &errBlob))) return E_FAIL;
        if (FAILED(D3DCompileFromFile(L"TesselletionPS.hlsl", nullptr, nullptr,
            "PSMain", "ps_5_0", flags, 0, &psBlob, &errBlob))) return E_FAIL;

        // �V�F�[�_�[����
        auto resultVS = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vs.ReleaseAndGetAddressOf());
        auto resultHS = device->CreateHullShader(hsBlob->GetBufferPointer(), hsBlob->GetBufferSize(), nullptr, hs.ReleaseAndGetAddressOf());
        auto resultDS = device->CreateDomainShader(dsBlob->GetBufferPointer(), dsBlob->GetBufferSize(), nullptr, ds.ReleaseAndGetAddressOf());
        auto resultPS = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, ps.ReleaseAndGetAddressOf());

        // ���̓��C�A�E�g (VS �� input)
        D3D11_INPUT_ELEMENT_DESC ie[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        device->CreateInputLayout(ie, _countof(ie),
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            inputLayout.GetAddressOf());

        // �O�p�`�̂R���_ (�R���g���[���|�C���g)
        struct VTX { float x, y, z; };
        VTX tri[3] = {
            { -0.5f, 0.0f, 0.0f },
            {  0.5f, 0.0f, 0.0f },
            {  0.0f, 0.8f, 0.0f },
        };
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(tri);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sd = { tri };
        device->CreateBuffer(&bd, &sd, vertexBuffer.ReleaseAndGetAddressOf());

        // �e�b�Z�W���p�萔�o�b�t�@
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CB_TESS);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        auto result = device->CreateBuffer(&bd, nullptr, g_cbTessFactor.ReleaseAndGetAddressOf());
        if (!m_rasterizerState)
        {
            D3D11_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.FillMode = D3D11_FILL_SOLID;
            rasterDesc.CullMode = D3D11_CULL_NONE; // �� �o�b�N�t�F�C�X�J�����O���ꎞ�I�ɃI�t��
            rasterDesc.FrontCounterClockwise = false;
            rasterDesc.DepthClipEnable = true;

            device->CreateRasterizerState(&rasterDesc, m_rasterizerState.ReleaseAndGetAddressOf());
        }
        return S_OK;
    }


  

}
;