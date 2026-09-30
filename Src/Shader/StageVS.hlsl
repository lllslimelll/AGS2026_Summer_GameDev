// VS/PS共通
#include "Common/VertexToPixelHeader.hlsli"

// IN
#include "Common/Vertex/VertexInputType.hlsli"
#define VERTEX_INPUT DX_MV1_VERTEX_TYPE_NMAP_1FRAME

// OUT
#define VS_OUTPUT VertexToPixelLit
#include "Common/Vertex/VertexShader3DHeader.hlsli"

// 定数バッファ
cbuffer cbUser : register(b7)
{
    // 基準座標(カメラ)
    float4 g_cameraPos; 
}

// Y座標を下げる基準距離
static const float CURVE_DIST = 5000.0f;
// CURVE_DIST での落下量
static const float CURVE_DROP = 1500.0f;
// 曲がり具合の係数
static const float CURVE_COEFF = CURVE_DROP / (CURVE_DIST * CURVE_DIST);

VS_OUTPUT main(VS_INPUT VSInput)
{
    VS_OUTPUT ret;

    // 座標変換
    float4 lLocalPosition;  // ローカル座標
    float4 lWorldPosition;  // ワールド座標
    float4 lViewPosition;   // ビュー座標

    // ローカル座標を取得
    lLocalPosition.xyz = VSInput.pos;
    lLocalPosition.w = 1.0f;

    // ローカル → ワールド
    lWorldPosition.w = 1.0f;
    lWorldPosition.xyz = mul(lLocalPosition, g_base.localWorldMatrix);

    // 頂点からカメラの水平距離に比例して、Y座標を下げる
    float2 dist = lWorldPosition.xz - g_cameraPos.xz; // 頂点からカメラの水平距離
    float heightOffset = (dist.x * dist.x + dist.y * dist.y) * CURVE_COEFF;
    lWorldPosition.y -= heightOffset;

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