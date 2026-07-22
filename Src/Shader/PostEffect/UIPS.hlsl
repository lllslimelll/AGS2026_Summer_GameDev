// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_param0; // x: 歪み強度
};

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    float2 uv = PSInput.uv;

    // 中心を原点に (-1 ~ 1)
    float2 local = uv * 2.0f - 1.0f;

    // バレル歪み
    float r2 = dot(local, local);
    float2 distorted = local * (1.0f + g_param0.x * r2);

    // UVに戻す
    float2 newUV = distorted * 0.5f + 0.5f;

    return diffuseMapTexture.Sample(diffuseMapSampler, newUV);
}