#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../Collider/ColliderBase.h"
#include "../../Collider/ColliderModel.h"
#include "Item.h"

Item::Item(GRADE grade, const VECTOR& pos)
    :
    ActorBase(),
	grade_(grade),
	defaultPos_(pos),
	state_(STATE::DROPPED),
    isAimed_(false)
{
	transform_.pos = pos;
}

Item::~Item()
{
}

void Item::Update(void)
{
	// 移動前座標を更新
	//prevPos_ = transform_.pos;

	// 各キャラクターごとの更新処理
	//UpdateProcess();

	// 重力による移動量
	//CalcGravityPow();

	// 衝突判定前準備
	//CollisionReserve();

	// 衝突判定
	//Collision();

	// モデル制御更新
	transform_.Update();

	// アニメーション再生
	//animController_->Update();

	// 各キャラクターごとの更新後処理
	//UpdateProcessPost();
}

void Item::Draw(void)
{
	// 基底クラス描画処理
	ActorBase::Draw();

#ifdef _DEBUG
    // 照準が当たっているときスフィアを緑、通常は赤で表示
    int color = isAimed_ ? 0x00ff00 : 0xff0000;
    DrawSphere3D(transform_.pos, 30.0f, 8, color, color, FALSE);
#endif
}

void Item::OnPickedUp(void)
{
}

void Item::OnThrow(const VECTOR& throwDir)
{
}

void Item::OnHitEnemy(void)
{
}

int Item::GetValue(void) const
{
    return 0;
}

Item::GRADE Item::GetGrade(void) const
{
    return grade_;
}

Item::STATE Item::GetState(void) const
{
    return state_;
}

bool Item::IsAimedBy(const VECTOR& rayOrigin, const VECTOR& rayEnd) const
{
    if (state_ != STATE::DROPPED) return false;

    // ① スフィア判定
    bool hit = AsoUtility::IsHitSphereCapsule(
        transform_.pos, 30.0f,
        rayOrigin, rayEnd, 0.0f);

    if (!hit) return false;

    // ② 遮蔽チェック
    for (const auto& c : hitColliders_)
    {
        if (c->GetShape() != ColliderBase::SHAPE::MODEL)
            continue;

        const ColliderModel* model =
            static_cast<const ColliderModel*>(c);

        auto result = model->GetNearestHitPolyLine(
            rayOrigin, transform_.pos);

        if (result.HitFlag > 0) return false;
    }

    return true;
}

void Item::SetAimed(bool aimed)
{
    isAimed_ = aimed;
}

void Item::InitLoad(void)
{

}

void Item::InitTransform(void)
{
}

void Item::InitCollider(void)
{
}

void Item::InitAnimation(void)
{
}

void Item::InitPost(void)
{
}

void Item::ChangeState(STATE state)
{
    if (state_ == state) return;
    state_ = state;

    switch (state_)
    {
    case STATE::DROPPED: ChangeStateDropped(); break;
    case STATE::HELD:    ChangeStateHeld();    break;
    case STATE::FLYING:  ChangeStateFlying();  break;
    default: break;
    }
}

void Item::ChangeStateDropped(void)
{
}

void Item::ChangeStateHeld(void)
{
}

void Item::ChangeStateFlying(void)
{
}

void Item::UpdateDropped(void)
{
}

void Item::UpdateHeld(void)
{
}

void Item::UpdateFlying(void)
{
}

void Item::UpdateThrowMove(void)
{
}

void Item::DrawBillboard(void) const
{
}
