#include <BufferHelpers.h>
#include <DirectXMath.h>
#include <vector>
#include <VertexTypes.h>
class Polygon
{
public:
    bool Initialize(ID3D11Device* device);//�o�b�t�@�E�V�F�[�_�[�̏�����
    void Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX world, DirectX::XMMATRIX viewProj);////�V�F�[�_�[�E�o�b�t�@�̃o�C���h

private:
    std::vector<DirectX::VertexPositionColor> vertices;
    std::vector<UINT> indices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    struct ConstantBufferData {
        DirectX::XMMATRIX worldViewProj;
    };
    DirectX::ConstantBuffer<ConstantBufferData> constantBuffer;
};