#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/Camera.h"
#include "../../../Manager/ResourceManager.h"
#include "../../Collider/ColliderBase.h"
#include "../../Collider/ColliderModel.h"
#include "Item.h"

Item::Item(const ItemData& data)
    :
    ActorBase(),
    type(data.type),
    grade_(data.grade),
    defaultPos_(data.defaultPos),
    value_(data.value),
    isAimed_(false)
{
}

Item::~Item()
{
}

void Item::Update(void)
{
	// 移動前座標を更新
	//prevPos_ = transform_.pos;

	// 更新処理
	//UpdateProcess();

	// 状態別更新
	stateUpdate_();

	// 重力による移動量
	//CalcGravityPow();

	// 衝突判定
	//Collision();

	// モデル制御更新
	transform_.Update();

	// 更新後処理
	//UpdateProcessPost();
}

void Item::Draw(void)
{
   // 選択中のアイテムのみ描画
   
	
	ActorBase::Draw();

#ifdef _DEBUG
   int color;

    if (isAimed_)
    {
        // 照準中は赤で統一
        color = 0xff0000;
    }
    else
    {
        // 種別ごとに色を変える
        switch (type)
        {
        case TYPE::COIN: color = 0xffff00; break; // 黄色
        case TYPE::GEM:  color = 0x00ffff; break; // 水色
        default:         color = 0xff0000; break; // 赤（未定義）
        }
    }

    DrawSphere3D(transform_.pos, 30.0f, 8, color, color, TRUE);
#endif
}

void Item::OnPickedUp(void)
{
	// アイテムを持っている状態に遷移
    ChangeState(STATE::HELD);
}

void Item::OnThrow(const VECTOR& throwDir)
{
    ChangeState(STATE::THROW);
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

    // スフィア判定
    bool hit = AsoUtility::IsHitSphereCapsule(
        transform_.pos, 30.0f,
        rayOrigin, rayEnd, 0.0f);

    if (!hit) return false;

    // 遮蔽チェック
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
    //// モデル読み込み
    //transform_.SetModel(resMng_.Dupulicate(			// 1個 = Load()  複数 = Depulicate()
    //    ResourceManager::SRC::MAIN_STAGE).handleId_);
}

void Item::InitTransform(void)
{
    // モデルの基本設定

    // 大きさ
    transform_.scl = AsoUtility::VECTOR_ONE;

    // モデル本来の向き
    transform_.quaRot = Quaternion::Identity();
    // ローカル回転
    transform_.quaRotLocal = Quaternion::Euler({ 0.0f, DX_PI_F / 180.0f, 0.0f });

    // 座標
    transform_.pos = defaultPos_;

    transform_.Update();
}

void Item::InitCollider(void)
{
}

void Item::InitAnimation(void)
{
}

void Item::InitPost(void)
{
	// 状態遷移の処理登録
    stateChanges_[STATE::DROPPED] = 
        std::bind(&Item::ChangeDropped, this);
    stateChanges_[STATE::HELD] =
        std::bind(&Item::ChangeHeld, this);
	stateChanges_[STATE::THROW] =
        std::bind(&Item::ChangeThrow, this);

    // 初期状態
    ChangeState(STATE::DROPPED);

}

void Item::ChangeState(STATE state)
{
    state_ = state;
    
	// 各状態遷移の処理
    stateChanges_[state_]();
}

void Item::ChangeDropped(void)
{
    stateUpdate_ = std::bind(&Item::UpdateDropped, this);
}

void Item::ChangeHeld(void)
{
    stateUpdate_ = std::bind(&Item::UpdateHeld, this);
}

void Item::ChangeThrow(void)
{
    stateUpdate_ = std::bind(&Item::UpdateThrow, this);
}

void Item::UpdateDropped(void)
{
}

void Item::UpdateHeld(void)
{
	// アイテムを持っているプレイヤーの座標に追従させる
	transform_.pos = VAdd(scnMng_.GetCamera()->GetTransform().pos, VGet(0.0f, 140.0f, 0.0f));
}

void Item::UpdateThrow(void)
{
}

void Item::UpdateThrowMove(void)
{
}

void Item::DrawBillboard(void) const
{
}
