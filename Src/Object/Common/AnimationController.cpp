#include <DxLib.h>
#include "../../Scene/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
    :
    modelId_(modelId),
    playType_(-1),
    playAnim_(),
    isLoop_(true),
    prevType_(-1),
    prevAnim_(),
    blendRate_(1.0f),
    blendTime_(0.2f)  // デフォルト0.2秒でブレンド
{
}

AnimationController::~AnimationController(void)
{
}

void AnimationController::Add(int type, float speed, const std::string path)
{
    Animation animation;
    animation.model = MV1LoadModel(path.c_str());
    animation.animIndex = -1;
    Add(type, speed, animation);
}

void AnimationController::AddInFbx(int type, float speed, int animIndex)
{
    Animation animation;
    animation.model = -1;
    animation.animIndex = animIndex;
    Add(type, speed, animation);
}

void AnimationController::Play(int type, bool isLoop)
{
    if (playType_ == type) { return; }

    // 前のアニメーションをブレンド用に保存
    if (playType_ != -1)
    {
        // 既にブレンド中なら前のブレンド先をデタッチ
        if (prevType_ != -1 && prevAnim_.attachNo != -1)
        {
            MV1DetachAnim(modelId_, prevAnim_.attachNo);
        }
        prevAnim_ = playAnim_;
        prevType_ = playType_;
        blendRate_ = 0.0f;  // ブレンド開始
    }
    else
    {
        blendRate_ = 1.0f;  // 最初の再生はブレンドなし
    }

    // 新しいアニメーションをセット
    playType_ = type;
    playAnim_ = animations_[type];
    playAnim_.step = 0.0f;
    isLoop_ = isLoop;

    Attach(playAnim_);
    playAnim_.totalTime = MV1GetAttachAnimTotalTime(modelId_, playAnim_.attachNo);

    // ブレンドウェイト初期設定
    if (prevType_ != -1)
    {
        MV1SetAttachAnimBlendRate(modelId_, prevAnim_.attachNo, 1.0f - blendRate_);
        MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, blendRate_);
    }
}

void AnimationController::Update(void)
{
    const float deltaTime = SceneManager::GetInstance().GetDeltaTime();

    // ブレンド更新
    if (blendRate_ < 1.0f && prevType_ != -1)
    {
        blendRate_ += deltaTime / blendTime_;
        if (blendRate_ >= 1.0f)
        {
            blendRate_ = 1.0f;
            // ブレンド完了：前のアニメーションをデタッチ
            if (prevAnim_.attachNo != -1)
            {
                MV1DetachAnim(modelId_, prevAnim_.attachNo);
                prevAnim_.attachNo = -1;
            }
            prevType_ = -1;
        }

        MV1SetAttachAnimBlendRate(modelId_, prevAnim_.attachNo, 1.0f - blendRate_);
        MV1SetAttachAnimBlendRate(modelId_, playAnim_.attachNo, blendRate_);

        // 前アニメのステップ更新
        if (prevAnim_.attachNo != -1)
        {
            prevAnim_.step += deltaTime * prevAnim_.speed;
            if (prevAnim_.step > prevAnim_.totalTime) { prevAnim_.step = 0.0f; }
            MV1SetAttachAnimTime(modelId_, prevAnim_.attachNo, prevAnim_.step);
        }
    }

    // 現在のアニメーション更新
    playAnim_.step += deltaTime * playAnim_.speed;

    if (playAnim_.step > playAnim_.totalTime)
    {
        if (isLoop_) { playAnim_.step = 0.0f; }
        else { playAnim_.step = playAnim_.totalTime; }
    }

    MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);
}

void AnimationController::Release(void)
{
    for (const std::pair<int, Animation>& pair : animations_)
    {
        if (pair.second.model != -1)
        {
            MV1DeleteModel(pair.second.model);
        }
    }
    animations_.clear();
}

int AnimationController::GetPlayType(void) const
{
    return playType_;
}

bool AnimationController::IsEnd(void) const
{
    if (isLoop_) return false;
    return playAnim_.step >= playAnim_.totalTime;
}

const AnimationController::Animation& AnimationController::GetPlayAnim(void) const
{
    return playAnim_;
}

bool AnimationController::IsBlending(void) const
{
    return blendRate_ < 1.0f;
}

void AnimationController::Add(int type, float speed, Animation& animation)
{
    animation.speed = speed;
    if (animations_.count(type) == 0)
    {
        animations_.emplace(type, animation);
    }
}

void AnimationController::Attach(Animation& anim)
{
    if (anim.model == -1)
    {
        anim.attachNo = MV1AttachAnim(modelId_, anim.animIndex);
    }
    else
    {
        anim.attachNo = MV1AttachAnim(modelId_, 0, anim.model);
    }
}