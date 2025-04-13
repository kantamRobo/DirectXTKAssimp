// Max boneêîÇ…âûÇ∂Çƒí≤êÆ
#define MAX_BONES 100


struct VS_INPUT {
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    uint4 BoneIDs : BONEIDS;
    float4 Weights : WEIGHTS;
};

cbuffer cbBones : register(b1) {
    matrix gBoneTransforms[MAX_BONES];
}

cbuffer cbPerObject : register(b0) {
    matrix worldViewProj;
}


struct VS_OUTPUT {
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input) {
    matrix skinMatrix = 0;

    for (int i = 0; i < 4; ++i) {
        skinMatrix += input.Weights[i] * gBoneTransforms[input.BoneIDs[i]];
    }

    VS_OUTPUT output;
    output.Pos = mul(float4(input.Pos, 1.0f), skinMatrix);
    output.Pos = mul(output.Pos, worldViewProj);
    output.UV = input.UV;
    return output;
}
