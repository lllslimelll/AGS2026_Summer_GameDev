// VS/PS共通
#include "Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

// PS
#include "Common/Pixel/PixelShader3DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_light_dir;
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{

    float4 color;

	// テクスチャーの色を取得
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f)
    {
        discard;
    }
    
    // 乗算合成
    color *= PSInput.diffuse;

    // ランバート反射
    float lightPow = saturate(dot(PSInput.normal, -PSInput.lightDir));
    color.rgb *= lightPow;
    
    // エミッシブを加算
    float emissive = 0.0f;
    color.rgb += emissive;
    
    // アンビエントを加算
    float ambient = 0.0f;
    color.rgb += ambient;
    
    return color;

}
