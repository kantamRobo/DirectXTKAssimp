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
    ID3D11ComputeShader* g_pFinalPassCS = NULL;
    ID3DBlob* pBlobFinalPassCS = nullptr;
    pd3dDevice->CreateComputeShader(pBlobFinalPassCS->GetBufferPointer(),
        pBlobFinalPassCS->GetBufferSize(),
        NULL, &g_pFinalPassCS);
}


void ComputeShader::CreateBuffer()
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(buffer);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags =
        D3D11_CPU_ACCESS_WRITE;


}

void ComputeShader::CreateUAV(ID3D11Device* device)
{
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_UNKNOWN;
    

}


