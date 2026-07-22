// VS/PS 共通
#include "Common/VertexToPixelHeader.hlsli"

// IN : Mixamo（法線マップ情報あり・5?8フレームの影響を受ける頂点）
#include "Common/Vertex/VertexInputType.hlsli"
#define VERTEX_INPUT DX_MV1_VERTEX_TYPE_NMAP_8FRAME

// OUT
#define VS_OUTPUT VertexToPixelLit
#include "Common/Vertex/VertexShader3DHeader.hlsli"

// スキニングメッシュ用 ローカル→ワールド変換行列配列（b3スロット）
// lwMatrix[boneIdx + 0..2] で1ボーン分の行0?行2を表す
#define L_W_MAT g_localWorldMatrix.lwMatrix

// 定数バッファ（b7スロット）
cbuffer cbUser : register(b7)
{
    float4 g_pivotPos; // xyz: ワールド湾曲の基準座標（カメラ位置）
};

// 湾曲量
static const float CURVE_AMOUNT = 0.00006f;

VS_OUTPUT main(VS_INPUT VSInput)
{
    VS_OUTPUT ret;

    // ========================================================
    //  スキンメッシュ用 ローカル→ワールド変換行列の合成
    //
    //  DxLib の仕様：blendIndices にはボーン行列の先頭行インデックスが
    //  あらかじめ「行番号 × 3」でパックされて渡ってくる。
    //  そのため +0 / +1 / +2 で同一ボーンの 行0・行1・行2 を参照できる。
    // ========================================================
    int4 lBoneIdx;
    float4 lWeight;
    float4 lL_W_Mat[3]; // 合成後の変換行列（行0・行1・行2）

    // ---- ボーン 1?4 : blendIndices0 / blendWeight0 ----
    lBoneIdx = VSInput.blendIndices0;
    lWeight = VSInput.blendWeight0;

    lL_W_Mat[0] = L_W_MAT[lBoneIdx.x + 0] * lWeight.xxxx;
    lL_W_Mat[1] = L_W_MAT[lBoneIdx.x + 1] * lWeight.xxxx;
    lL_W_Mat[2] = L_W_MAT[lBoneIdx.x + 2] * lWeight.xxxx;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.y + 0] * lWeight.yyyy;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.y + 1] * lWeight.yyyy;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.y + 2] * lWeight.yyyy;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.z + 0] * lWeight.zzzz;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.z + 1] * lWeight.zzzz;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.z + 2] * lWeight.zzzz;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.w + 0] * lWeight.wwww;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.w + 1] * lWeight.wwww;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.w + 2] * lWeight.wwww;

    // ---- ボーン 5?8 : blendIndices1 / blendWeight1 ----
    lBoneIdx = VSInput.blendIndices1;
    lWeight = VSInput.blendWeight1;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.x + 0] * lWeight.xxxx;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.x + 1] * lWeight.xxxx;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.x + 2] * lWeight.xxxx;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.y + 0] * lWeight.yyyy;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.y + 1] * lWeight.yyyy;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.y + 2] * lWeight.yyyy;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.z + 0] * lWeight.zzzz;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.z + 1] * lWeight.zzzz;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.z + 2] * lWeight.zzzz;

    lL_W_Mat[0] += L_W_MAT[lBoneIdx.w + 0] * lWeight.wwww;
    lL_W_Mat[1] += L_W_MAT[lBoneIdx.w + 1] * lWeight.wwww;
    lL_W_Mat[2] += L_W_MAT[lBoneIdx.w + 2] * lWeight.wwww;
    // ========================================================

    // ---- 頂点座標 : ローカル → ワールド（スキニング版） ----
    float4 lLocalPosition;
    lLocalPosition.xyz = VSInput.pos;
    lLocalPosition.w = 1.0f;

    float4 lWorldPosition;
    lWorldPosition.w = 1.0f;
    lWorldPosition.x = dot(lLocalPosition, lL_W_Mat[0]);
    lWorldPosition.y = dot(lLocalPosition, lL_W_Mat[1]);
    lWorldPosition.z = dot(lLocalPosition, lL_W_Mat[2]);

    // ---- ワールド湾曲（WorldBending）----
    // カメラ基準の XZ 距離に応じて Y を下げ、球面ワールドを演出する
    float2 offset = lWorldPosition.xz - g_pivotPos.xz;
    float bend = (offset.x * offset.x + offset.y * offset.y) * CURVE_AMOUNT;
    lWorldPosition.y -= bend;

    // ---- ワールド → ビュー ----
    float4 lViewPosition;
    lViewPosition.w = 1.0f;
    lViewPosition.xyz = mul(lWorldPosition, g_base.viewMatrix);
    ret.vwPos.xyz = lViewPosition.xyz;

    // ---- ビュー → スクリーン（射影変換） ----
    ret.svPos = mul(lViewPosition, g_base.projectionMatrix);

    // ---- ピクセルシェーダへの引き継ぎ ----
    ret.uv.x = VSInput.uv0.x;
    ret.uv.y = VSInput.uv0.y;
    ret.worldPos = lWorldPosition.xyz;

    // ---- 法線 : ローカル → ワールド（スキニング版） ----
    // 法線は方向ベクトルなので平行移動成分（w列）は不要 → .xyz のみ使う
    float3 lWorldNormal;
    lWorldNormal.x = dot(VSInput.norm, lL_W_Mat[0].xyz);
    lWorldNormal.y = dot(VSInput.norm, lL_W_Mat[1].xyz);
    lWorldNormal.z = dot(VSInput.norm, lL_W_Mat[2].xyz);
    ret.normal = normalize(lWorldNormal);

    ret.diffuse = VSInput.diffuse;
    ret.lightDir = float3(0.3f, -0.7f, 0.8f);
    ret.lightAtPos = float3(0.0f, 0.0f, 0.0f);

    return ret;
}
