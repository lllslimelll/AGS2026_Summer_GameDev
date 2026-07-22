// PS
#include "../Common/Pixel/PixelShader2DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_param0; // x: 横シアー量, y: 縦シアー量
};

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    float2 uv = PSInput.uv;


    return tex.Sample(texSampler, uv);
}