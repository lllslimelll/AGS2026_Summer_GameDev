#include <algorithm>
#include <cstring>
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
    SetupCollision();
}

// コリジョン情報のセットアップ
// 対象フレームが指定されていればフレーム単位で構築する
void StaticMeshComponent::SetupCollision(void)
{
    isCollSetup_ = false;

    if (modelId_ == -1) return;

    // コリジョンに参加しないなら重いセットアップは行わない
    if (GetProfile().type_ == CollisionProfileType::NO_COLLISION) return;

    if (includeFrameIds_.empty())
    {
        MV1SetupCollInfo(modelId_);
    }
    else
    {
        for (int frame : includeFrameIds_)
        {
            MV1SetupCollInfo(modelId_, frame);
        }
    }
    isCollSetup_ = true;
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
    hasPrevMat_ = false;

    SetupCollision();
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

// 当たり判定対象フレームの一覧を取得する
std::vector<int> StaticMeshComponent::GetCollisionFrames(void) const
{
    if (includeFrameIds_.empty())
    {
        // 未指定ならモデル全体（-1）
        return { -1 };
    }
    return includeFrameIds_;
}

void StaticMeshComponent::Update(void)
{
    if (modelId_ == -1) return;

    // ワールド行列をモデルに反映
    Matrix4x4 worldMat = GetWorldMat();
    MATRIX dxMat;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            dxMat.m[i][j] = worldMat.m[i][j];

    MV1SetMatrix(modelId_, dxMat);

    // コリジョン情報の更新（重いので行列が変わったときだけ）
    if (!isCollSetup_) return;

    bool isMoved =
        !hasPrevMat_ ||
        std::memcmp(&prevMat_, &dxMat, sizeof(MATRIX)) != 0;

    if (isMoved)
    {
        if (includeFrameIds_.empty())
        {
            MV1RefreshCollInfo(modelId_);
        }
        else
        {
            for (int frame : includeFrameIds_)
            {
                MV1RefreshCollInfo(modelId_, frame);
            }
        }
        prevMat_ = dxMat;
        hasPrevMat_ = true;
    }
}

// ---------------------------------------------------------------
// 描画
// ---------------------------------------------------------------

void StaticMeshComponent::Draw(void)
{
    if (modelId_ == -1) return;

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