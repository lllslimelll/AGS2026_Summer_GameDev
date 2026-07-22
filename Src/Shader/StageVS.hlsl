// VS/PS共通
#include "Common/VertexToPixelHeader.hlsli"

// IN
#include "Common/Vertex/VertexInputType.hlsli"
#define VERTEX_INPUT DX_MV1_VERTEX_TYPE_NMAP_1FRAME

// OUT
#define VS_OUTPUT VertexToPixelLit
#include "Common/Vertex/VertexShader3DHeader.hlsli"

//定数バッファ
cbuffer cbUser : register(b7)
{
    float4 g_pivotPos; // xyz: 基準座標(カメラ)
}

// 曲率
static const float CURVE_AMOUNT = 0.00006f;

VS_OUTPUT main(VS_INPUT VSInput)
{
    VS_OUTPUT ret;

    float4 lLocalPosition;
    float4 lWorldPosition;
    float4 lViewPosition;

    lLocalPosition.xyz = VSInput.pos;
    lLocalPosition.w = 1.0f;

    // ローカル → ワールド
    lWorldPosition.w = 1.0f;
    lWorldPosition.xyz = mul(lLocalPosition, g_base.localWorldMatrix);

    // ===== World Bending =====
    float2 offset = lWorldPosition.xz - g_pivotPos.xz;
    float bend = (offset.x * offset.x + offset.y * offset.y) * CURVE_AMOUNT;
    lWorldPosition.y -= bend;
    // =========================

    // ワールド → ビュー
    lViewPosition.w = 1.0f;
    lViewPosition.xyz = mul(lWorldPosition, g_base.viewMatrix);
    ret.vwPos.xyz = lViewPosition.xyz;

    // ビュー → プロジェクション
    ret.svPos = mul(lViewPosition, g_base.projectionMatrix);

    ret.uv.x = VSInput.uv0.x;
    ret.uv.y = VSInput.uv0.y;
    ret.worldPos = lWorldPosition.xyz;
    // 法線をローカル空間からワールド空間へ変換
    ret.normal = normalize(mul(VSInput.norm, (float3x3) g_base.localWorldMatrix));
    ret.diffuse = VSInput.diffuse;
    ret.lightDir = float3(0.3f, -0.7f, 0.8f);
    ret.lightAtPos = float3(0.0f, 0.0f, 0.0f);

    return ret;
}