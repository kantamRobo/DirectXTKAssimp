#include "pch.h"
#include "ComputeShader.h"
#include <d3dcompiler.h>
#include <string>

// HLSL ファイルの検索ヘルパー (カレントディレクトリ、exeと同じディレクトリ、プロジェクトディレクトリなどを探索)
static std::wstring FindShaderPath(const std::wstring& filename)
{
    if (GetFileAttributesW(filename.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        return filename;
    }

    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash)
    {
        *lastSlash = L'\0';
    }

    // 候補1: exe と同階層
    std::wstring candidate1 = std::wstring(exePath) + L"\\" + filename;
    if (GetFileAttributesW(candidate1.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        return candidate1;
    }

    // 候補2: プロジェクトディレクトリ (x64/Debug から 2階層上)
    std::wstring candidate2 = std::wstring(exePath) + L"\\..\\..\\" + filename;
    if (GetFileAttributesW(candidate2.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        return candidate2;
    }

    return filename;
}

void ComputeShader::CreateShader(ID3D11Device* device)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif
    // Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
    LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
    const D3D_SHADER_MACRO defines[] =
    {
        "EXAMPLE_DEFINE", "1",
        NULL, NULL
    };

    auto entryPoint = "main";
    std::wstring shaderPath = FindShaderPath(L"ComputeShader.hlsl");

    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        shaderPath.c_str(),
        defines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint,
        profile,
        flags,
        0,
        pBlobFinalPassCS.ReleaseAndGetAddressOf(),
        errorBlob.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        else
        {
            char msg[256];
            sprintf_s(msg, "ComputeShader compile failed. Path: %ls, hr: 0x%08X\n", shaderPath.c_str(), static_cast<unsigned int>(hr));
            OutputDebugStringA(msg);
        }
        DX::ThrowIfFailed(hr);
    }
}

void ComputeShader::CreateComputeShader(ID3D11Device* pd3dDevice)
{
    if (!pBlobFinalPassCS)
    {
        return;
    }

    HRESULT hr = pd3dDevice->CreateComputeShader(
        pBlobFinalPassCS->GetBufferPointer(),
        pBlobFinalPassCS->GetBufferSize(),
        nullptr,
        g_pFinalPassCS.ReleaseAndGetAddressOf()
    );
    DX::ThrowIfFailed(hr);
}

void ComputeShader::CreateBuffer(DX::DeviceResources* DR)
{
    auto device = DR->GetD3DDevice();

    // 構造化バッファの作成
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(rwStructuredBuffer); // 要素数 1
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(rwStructuredBuffer);

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &RWbuffer;

    HRESULT hr = device->CreateBuffer(&desc, &initData, outputbuffer.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(hr);

    // 結果表示・確認用ステージングバッファの作成
    D3D11_BUFFER_DESC readDesc = {};
    readDesc.ByteWidth = desc.ByteWidth;
    readDesc.Usage = D3D11_USAGE_STAGING;
    readDesc.BindFlags = 0;
    readDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    readDesc.MiscFlags = 0;

    hr = device->CreateBuffer(&readDesc, nullptr, ReadbackBuffer.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(hr);
}

void ComputeShader::CreateUAV(ID3D11Device* device)
{
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = 1;

    HRESULT hr = device->CreateUnorderedAccessView(outputbuffer.Get(), &uavDesc, uav.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(hr);
}

void ComputeShader::Dispatch(DX::DeviceResources* DR)
{
    auto context = DR->GetD3DDeviceContext();

    // 1. バインドしてディスパッチ
    ID3D11UnorderedAccessView* uavs[] = { uav.Get() };
    context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);
    context->CSSetShader(g_pFinalPassCS.Get(), nullptr, 0);

    // バッファ要素数 1、シェーダーの numthreads が (1, 1, 1) なので 1 スレッドグループを実行
    context->Dispatch(1, 1, 1);

    // 2. パイプライン競合を防ぐため UAV と CS をアンバインド
    ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    context->CSSetShader(nullptr, nullptr, 0);

    // 3. データをステージングバッファにコピー
    context->CopyResource(ReadbackBuffer.Get(), outputbuffer.Get());

    // 4. マップして計算結果を取得・確認
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = context->Map(ReadbackBuffer.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        auto data = static_cast<const rwStructuredBuffer*>(mapped.pData);

        // 動作確認用デバッグ出力 (最初の10フレーム、および60フレームごとに出力)
        static int frameCount = 0;
        if (frameCount < 10 || frameCount % 60 == 0)
        {
            char buff[128];
            sprintf_s(buff, "[ComputeShader Result] Frame %d: index = %u, value = %.1f\n", frameCount, data->index, data->value);
            OutputDebugStringA(buff);
        }
        frameCount++;

        // 必ず Unmap を呼び出してマップ状態を解除する
        context->Unmap(ReadbackBuffer.Get(), 0);
    }
}

void ComputeShader::ReleaseResources()
{
    uav.Reset();
    outputbuffer.Reset();
    ReadbackBuffer.Reset();
    g_pFinalPassCS.Reset();
    pBlobFinalPassCS.Reset();
}

