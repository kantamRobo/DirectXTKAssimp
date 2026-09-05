struct VSInput
{
    float3 aPos : POSITION; // location = 0
    float3 aNormal : NORMAL; // location = 1
    float2 aTexCoord : TEXCOORD0; // location = 2
    uint4 aJointNum : BLENDINDICES; // location = 3 (uvec4)
    float4 aJointWeight : BLENDWEIGHT; // location = 4 (vec4)
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

// スロット 0: カメラ行列 (b0)
cbuffer CameraBuffer : register(b0)
{
    float4x4 gWorldViewProj;
};

// スロット 1: ボーン行列配列 (b1)
cbuffer JointMatrices : register(b1)
{
    float4x4 gJointMat[128];
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    // スキニング行列の合成
    float4x4 skinMat = input.aJointWeight.x * gJointMat[input.aJointNum.x] +
                       input.aJointWeight.y * gJointMat[input.aJointNum.y] +
                       input.aJointWeight.z * gJointMat[input.aJointNum.z] +
                       input.aJointWeight.w * gJointMat[input.aJointNum.w];
    
    // スキニング後のローカル座標計算
    float4 skinnedPos = mul(float4(input.aPos, 1.0f), skinMat);
    
    // 投影変換
    output.position = mul(skinnedPos, gWorldViewProj);
    
    output.normal = input.aNormal;
    output.texCoord = input.aTexCoord;
    
    return output;
}