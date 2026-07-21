// Component/StaticMeshComponent.h
#pragma once
#include <string>
#include <vector>
#include <DxLib.h>
#include "../Core/Vector3.h"
#include "PrimitiveComponent.h"

// モデル形状の当たり判定 + 描画 Component
// Unreal の UStaticMeshComponent に相当
class StaticMeshComponent : public PrimitiveComponent
{
public:

    StaticMeshComponent(
        ActorBase& owner,
        int modelId);

    ~StaticMeshComponent(void) override;

    // ---------------------------------------------------------------
    // ライフサイクル
    // ---------------------------------------------------------------

    // 初期化時にモデルの当たり判定情報をセットアップ
    void Init(void)    override;

    // ---------------------------------------------------------------
    // 形状
    // ---------------------------------------------------------------

    SHAPE GetShape(void) const override { return SHAPE::STATIC_MESH; }

    // ---------------------------------------------------------------
    // モデル
    // ---------------------------------------------------------------

    int  GetModelId(void) const;
    void SetModelId(int modelId);

    // ---------------------------------------------------------------
    // 除外フレーム（当たり判定から除外するフレーム）
    // ---------------------------------------------------------------

    void AddExcludeFrame(const std::string& name);
    void ClearExcludeFrames(void);
    bool IsExcludeFrame(int frameIndex) const;

    // ---------------------------------------------------------------
    // 対象フレーム（当たり判定対象のフレーム）
    // ---------------------------------------------------------------

    void AddIncludeFrame(const std::string& name);
    void ClearIncludeFrames(void);
    bool IsIncludeFrame(int frameIndex) const;

    // 当たり判定対象フレームの一覧を取得する
    // （対象フレーム未指定なら { -1 } = モデル全体）
    std::vector<int> GetCollisionFrames(void) const;

    // 更新
    void Update(void) override;

    // ---------------------------------------------------------------
    // 描画
    // ---------------------------------------------------------------

    void Draw(void) override;

private:

    // モデルハンドル
    int modelId_;

    // 除外フレームインデックス
    std::vector<int> excludeFrameIds_;

    // 対象フレームインデックス
    std::vector<int> includeFrameIds_;

    // コリジョン情報が既にセットアップ済みか
    bool isCollSetup_ = false;

    // 前回のワールド行列（動いたときだけコリジョン情報を更新するため）
    MATRIX prevMat_;
    bool   hasPrevMat_ = false;

    // コリジョン情報のセットアップ
    void SetupCollision(void);

    // フレーム名からインデックスを取得して登録する
    void RegisterFrameIds(
        const std::string& name,
        std::vector<int>& frameIds);
};