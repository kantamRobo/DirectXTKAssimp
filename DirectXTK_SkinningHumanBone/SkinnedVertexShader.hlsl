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

// set = 1, binding = 0
cbuffer Matrices : register(b0, space1)
{
    float4x4 view;
    float4x4 projection;
};

// std140, set = 2, binding = 0
cbuffer JointMatrices : register(b0, space2)
{
    float4x4 jointMat;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    // Calculate composite Linear Blend Skinning matrix
    float4x4 skinMat = input.aJointWeight.x * jointMat[input.aJointNum.x] +
                       input.aJointWeight.y * jointMat[input.aJointNum.y] +
                       input.aJointWeight.z * jointMat[input.aJointNum.z] +
                       input.aJointWeight.w * jointMat[input.aJointNum.w];
    
    // Transform vertex position: projection * view * skinMat * vec4(aPos, 1.0)
    float4 skinnedPos = mul(skinMat, float4(input.aPos, 1.0));
    output.position = mul(projection, mul(view, skinnedPos));
    
    output.normal = input.aNormal;
    output.texCoord = input.aTexCoord;
    
    return output;
}
