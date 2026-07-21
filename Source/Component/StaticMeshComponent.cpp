#include <algorithm>
#include "StaticMeshComponent.h"

StaticMeshComponent::StaticMeshComponent(
    ActorBase& owner,
    int modelId)
    : PrimitiveComponent(owner)
    , modelId_(modelId)
{
}

StaticMeshComponent::~StaticMeshComponent(void)
{
}

// ---------------------------------------------------------------
// ライフサイクル
// ---------------------------------------------------------------

void StaticMeshComponent::Init(void)
{
    // 親クラスの Init を呼ぶ（CollisionManager に登録）
    PrimitiveComponent::Init();

    // モデルの当たり判定情報をセットアップ
    if (modelId_ != -1)
    {
        MV1SetupCollInfo(modelId_);
    }
}

// ---------------------------------------------------------------
// モデル
// ---------------------------------------------------------------

int StaticMeshComponent::GetModelId(void) const
{
    return modelId_;
}

void StaticMeshComponent::SetModelId(int modelId)
{
    modelId_ = modelId;

    if (modelId_ != -1)
    {
        MV1SetupCollInfo(modelId_);
    }
}

// ---------------------------------------------------------------
// 除外フレーム
// ---------------------------------------------------------------

void StaticMeshComponent::AddExcludeFrame(const std::string& name)
{
    RegisterFrameIds(name, excludeFrameIds_);
}

void StaticMeshComponent::ClearExcludeFrames(void)
{
    excludeFrameIds_.clear();
}

bool StaticMeshComponent::IsExcludeFrame(int frameIndex) const
{
    return std::find(
        excludeFrameIds_.begin(),
        excludeFrameIds_.end(),
        frameIndex) != excludeFrameIds_.end();
}

// ---------------------------------------------------------------
// 対象フレーム
// ---------------------------------------------------------------

void StaticMeshComponent::AddIncludeFrame(const std::string& name)
{
    RegisterFrameIds(name, includeFrameIds_);
}

void StaticMeshComponent::ClearIncludeFrames(void)
{
    includeFrameIds_.clear();
}

bool StaticMeshComponent::IsIncludeFrame(int frameIndex) const
{
    return std::find(
        includeFrameIds_.begin(),
        includeFrameIds_.end(),
        frameIndex) != includeFrameIds_.end();
}

// ---------------------------------------------------------------
// 描画
// ---------------------------------------------------------------

void StaticMeshComponent::Draw(void)
{
    if (modelId_ == -1) return;

    // ワールド行列をモデルに適用
    Matrix4x4 worldMat = GetWorldMat();

    // Matrix4x4 → MATRIX に変換して DxLib に渡す
    MATRIX dxMat;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            dxMat.m[i][j] = worldMat.m[i][j];

    MV1SetMatrix(modelId_, dxMat);
    MV1DrawModel(modelId_);
}

// ---------------------------------------------------------------
// フレーム名からインデックスを取得して登録する
// ---------------------------------------------------------------

void StaticMeshComponent::RegisterFrameIds(
    const std::string& name,
    std::vector<int>& frameIds)
{
    if (modelId_ == -1) return;

    int frameNum = MV1GetFrameNum(modelId_);
    for (int i = 0; i < frameNum; i++)
    {
        std::string frameName = MV1GetFrameName(modelId_, i);
        if (frameName.find(name) != std::string::npos)
        {
            frameIds.push_back(i);
        }
    }
}