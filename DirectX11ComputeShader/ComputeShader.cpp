#include "pch.h"
#include "ComputeShader.h"
#include <d3dcompiler.h>
#include <string>
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
    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(L"ComputeShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint, profile,
        flags, 0, &shaderBlob, &errorBlob);
}

void ComputeShader::CreateComputeShader(ID3D11Device* pd3dDevice)
{
  
    ID3DBlob* pBlobFinalPassCS = nullptr;
    pd3dDevice->CreateComputeShader(pBlobFinalPassCS->GetBufferPointer(),
        pBlobFinalPassCS->GetBufferSize(),
        NULL, &g_pFinalPassCS);
}


void ComputeShader::CreateBuffer(DX::DeviceResources* DR)
{
    auto device = DR->GetD3DDevice();
    auto context = DR->GetD3DDeviceContext();
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(RWbuffer);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS|D3D11_BIND_SHADER_RESOURCE;
   
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(RWStructuredBuffer);
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &RWbuffer;

    device->CreateBuffer(&desc, &initData, &buffer);
    

    //結果を表示する読み取り専用バッファの作成
    D3D11_BUFFER_DESC readDesc = desc;
    readDesc.Usage = D3D11_USAGE_STAGING;
    readDesc.BindFlags = 0;
    readDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    readDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    device->CreateBuffer(&readDesc, nullptr, &ReadbackBuffer);

    


}

void ComputeShader::CreateUAV(ID3D11Device* device)
{
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = sizeof(RWbuffer);

    device->CreateUnorderedAccessView(buffer, &uavDesc, &uav);

}


void ComputeShader::Dispatch(DX::DeviceResources* DR)
{
    auto context = DR->GetD3DDeviceContext();
    // 6. バインドしてディスパッチ
    context->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    context->CSSetShader(g_pFinalPassCS, nullptr, 0);
    context->Dispatch(sizeof(RWbuffer), 1, 1);
//データ反映
    context->CopyResource(ReadbackBuffer, buffer);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    context->Map(ReadbackBuffer, 0, D3D11_MAP_READ, 0, &mapped);
    //context->Unmap(ReadbackBuffer, 0);

}

