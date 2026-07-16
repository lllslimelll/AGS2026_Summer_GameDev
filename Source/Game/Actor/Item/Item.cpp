#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../Scene/SceneManager.h"
#include "../../Camera/Camera.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SoundManager.h"
#include "../../../Collision/ColliderBase.h"
#include "../../../Collision/ColliderSphere.h"
#include "Item.h"

void Item::SetCameraPos(const VECTOR& pos)
{
	camPos_ = pos;
}

Item::Item(const ItemData& data)
    :
    ActorBase(),
    type_(data.type),
    grade_(data.grade),
    defaultPos_(data.defaultPos),
    value_(data.value),
    valueBillImg_{ -1, -1, -1 },
    isAimed_(false),
    isSelected_(false)
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
    // モデル描画
   //if (transform_.modelId != -1) return;

   //MV1DrawModel(transform_.modelId);


	// 落ちてる状態
    bool shouldDraw = (state_ == STATE::DROPPED);

    if (shouldDraw)
    {
        ActorBase::Draw();
    }

    // 状態別描画
    stateDraw_();

#ifdef _DEBUG

    // 所有しているコライダの描画
    for (const auto& own : ownColliders_)
    {
        //own.second->Draw();
    }
#endif
}

void Item::OnPickedUp(void)
{
    SoundManager::GetInstance().PlayPickUp();
	// アイテムを持っている状態に遷移
    ChangeState(STATE::HELD);
}

void Item::OnThrow(const VECTOR& throwDir)
{
    ChangeState(STATE::THROW);
}

void Item::OnDelivered(void)
{
    ChangeState(STATE::DELIVERED);
}

void Item::OnHitEnemy(void)
{
}

Item::TYPE Item::GetType(void) const
{
    return type_;
}

int Item::GetValue(void) const
{
    return value_;
}

Item::GRADE Item::GetGrade(void) const
{
    return grade_;
}

Item::STATE Item::GetState(void) const
{
    return state_;
}

bool Item::IsAimed(const VECTOR& rayOrigin, const VECTOR& rayEnd) const
{
    if (state_ != STATE::DROPPED) return false;

    // 一旦AsoUtilityで確認
    bool hit = AsoUtility::IsHitSphereCapsule(
        transform_.pos, 30.0f,
        rayOrigin, rayEnd, 0.0f);

    return hit;
}

void Item::SetAimed(bool aimed)
{
    isAimed_ = aimed;
}

void Item::SetSelected(bool selected)
{
	isSelected_ = selected;
}

void Item::SetHeldPos(const VECTOR& pos)
{
    if (state_ != STATE::HELD) return;
    transform_.pos = pos;
    transform_.Update();
}

void Item::SetNouhin(bool nouhin)
{
    isNouhinn_ = nouhin;
}

void Item::OnDrop(const VECTOR& pos)
{
    // プレイヤーの足元座標をセット
    transform_.pos = pos;  

    VECTOR up = VNorm(VSub(pos, { 0.0f, 0.0f, 0.0f }));
    VECTOR forward = AsoUtility::DIR_F;
    float dot = VDot(forward, up);
    if (fabsf(dot) > 0.99f) { forward = AsoUtility::DIR_R; }
    forward = VNorm(VSub(forward, VScale(up, dot)));
    transform_.quaRot = Quaternion::LookRotation(forward, up);

    transform_.Update();
    ChangeState(STATE::DROPPED);
}

void Item::InitLoad(void)
{
    //// モデル読み込み
    //transform_.SetModel(resMng_.Dupulicate(			// 1個 = Load()  複数 = Depulicate()
    //    ResourceManager::SRC::MAIN_STAGE).handleId_);

    switch (type_)
    {
    case TYPE::type1:
        transform_.SetModel(MV1LoadModel("Data/Model/Item/crystal1.mv1"));
        break;
    case TYPE::type2:
        transform_.SetModel(MV1LoadModel("Data/Model/Item/crystal2.mv1"));
        break;
    //case TYPE::type3:
    //    transform_.SetModel(MV1LoadModel("Data/Model/Item/crystal3.mv1"));
    //    break;
    }
}

void Item::InitTransform(void)
{
    // モデルの基本設定

    // 大きさ
    transform_.scl = { 50.0f,50.0f ,50.0f };

    // モデル本来の向き
    transform_.quaRot = Quaternion::Identity();
    // ローカル回転
    transform_.quaRotLocal = Quaternion::Euler({ 0.0f, DX_PI_F / 180.0f, 0.0f });

    // 座標
    transform_.pos = defaultPos_;

    VECTOR upVec = VNorm((VSub(transform_.pos, { 0,0,0 })));
    transform_.pos = VAdd(transform_.pos, VScale(upVec, 34));

    // 月面の法線方向を上として向きを設定
    VECTOR up = VNorm(VSub(transform_.pos, { 0.0f, 0.0f, 0.0f }));
    VECTOR forward = AsoUtility::DIR_F;
    float dot = VDot(forward, up);
    if (fabsf(dot) > 0.99f) { forward = AsoUtility::DIR_R; }
    forward = VNorm(VSub(forward, VScale(up, dot)));
    transform_.quaRot = Quaternion::LookRotation(forward, up);

    transform_.Update();
}

void Item::InitCollider(void)
{
    // カプセルコライダ
    ColliderSphere* colSphere = new ColliderSphere(
        CollisionProfileType::ITEM,
        this,
        AsoUtility::VECTOR_ZERO,
        50.0f);

    RegisterCollider(colSphere, static_cast<int>(COLLIDER_TYPE::SPHERE));
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
    stateChanges_[STATE::DELIVERED] =
        std::bind(&Item::ChangeDelivered, this);

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
    stateDraw_ = std::bind(&Item::DrawDropped, this);
}

void Item::ChangeHeld(void)
{
    stateUpdate_ = std::bind(&Item::UpdateHeld, this);
    stateDraw_ = std::bind(&Item::DrawHeld, this);
}

void Item::ChangeThrow(void)
{
    stateUpdate_ = std::bind(&Item::UpdateThrow, this);
    stateDraw_ = std::bind(&Item::DrawThrow, this);
}

void Item::ChangeDelivered(void)
{
    stateUpdate_ = std::bind(&Item::UpdateDelivered, this);
    stateDraw_ = std::bind(&Item::DrawDelelivered, this);
}

void Item::UpdateDropped(void)
{
}

void Item::UpdateHeld(void)
{
}

void Item::UpdateThrow(void)
{
}

void Item::UpdateDelivered(void)
{

}

void Item::DrawDropped(void)
{
	// 標準に当たっている時だけ価値のビルボード画像描画
    if (isAimed_)
    {
        // ビルボード画像の代わりにデバッグ文字で価値を表示
        // 3D座標をスクリーン座標に変換
        VECTOR screenPos = ConvWorldPosToScreenPos(transform_.pos);

        // カメラの後ろにある場合は描画しない（z > 1.0f）
        if (screenPos.z <= 1.0f)
        {
            int prevSize = GetFontSize();
            SetFontSize(32);
            DrawFormatString(
                (int)screenPos.x,
                (int)screenPos.y,
                GetColor(255, 255, 255),
                "$%d", value_);
            SetFontSize(prevSize);
        }
    }
}

void Item::DrawHeld(void)
{
    
}

void Item::DrawThrow(void)
{
  
}

void Item::DrawDelelivered(void)
{
}

void Item::UpdateThrowMove(void)
{
}

void Item::DrawBillboard(void) const
{
    VECTOR Vec = VNorm(VSub(camPos_, transform_.pos));

    switch (type_)
    {
    case TYPE::type1:
        DrawBillboard3D(VAdd(transform_.pos, VScale(Vec, 32.0f)), 0.5f, 0.5f, 50.0f, 0.0f, valueBillImg_[0], true);
        break;
	case TYPE::type2:
        DrawBillboard3D(VAdd(transform_.pos, VScale(Vec, 32.0f)), 0.5f, 0.5f, 50.0f, 0.0f, valueBillImg_[1], true);
		break;
	case TYPE::type3:
		DrawBillboard3D(VAdd(transform_.pos, VScale(Vec, 32.0f)), 0.5f, 0.5f, 50.0f, 0.0f, valueBillImg_[2], true);
		break;
    }
}
