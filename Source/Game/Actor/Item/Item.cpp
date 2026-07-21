#include <DxLib.h>
#include "../../../Utility/AsoUtility.h"
#include "../../Scene/SceneManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SoundManager.h"
#include "../../../Component/SphereComponent.h"
#include "../../../Component/StaticMeshComponent.h"
#include "Item.h"

void Item::SetCameraPos(const Vector3& pos) { camPos_ = pos; }

Item::Item(const ItemData& data)
    : ActorBase()
    , type_(data.type)
    , grade_(data.grade)
    , defaultPos_(data.defaultPos)
    , value_(data.value)
{
}

Item::~Item() {}

void Item::Update(void)
{
    stateUpdate_();
    ActorBase::Update();
}

void Item::Draw(void)
{
    if (state_ == STATE::DROPPED)
        ActorBase::Draw();
    stateDraw_();
}

void Item::OnPickedUp(void)
{
    SoundManager::GetInstance().PlayPickUp();
    ChangeState(STATE::HELD);
}

void Item::OnThrow(const Vector3& throwDir) { ChangeState(STATE::THROW); }
void Item::OnDelivered(void) { ChangeState(STATE::DELIVERED); }
void Item::OnHitEnemy(void) {}

Item::TYPE  Item::GetType(void)  const { return type_; }
int         Item::GetValue(void) const { return value_; }
Item::GRADE Item::GetGrade(void) const { return grade_; }
Item::STATE Item::GetState(void) const { return state_; }

bool Item::IsAimed(const Vector3& rayOrigin, const Vector3& rayEnd) const
{
    if (state_ != STATE::DROPPED) return false;
    return AsoUtility::IsHitSphereCapsule(GetPos().ToVECTOR(), 30.0f, rayOrigin.ToVECTOR(), rayEnd.ToVECTOR(), 0.0f);
}

void Item::SetAimed(bool aimed) { isAimed_ = aimed; }
void Item::SetSelected(bool selected) { isSelected_ = selected; }
void Item::SetNouhin(bool nouhin) { isNouhinn_ = nouhin; }

void Item::SetHeldPos(const Vector3& pos)
{
    if (state_ != STATE::HELD) return;
    SetPos(pos);
}

void Item::OnDrop(const Vector3& pos)
{
    SetPos(pos);

    Vector3 up = pos.Normalized();
    Vector3 fwd = Vector3::FORWARD;
    if (fabsf(Vector3::Dot(fwd, up)) > 0.99f) fwd = Vector3::RIGHT;
    fwd = (fwd - up * Vector3::Dot(fwd, up)).Normalized();
    SetRot(Quaternion::LookRotation(fwd, up));

    ChangeState(STATE::DROPPED);
}

void Item::Init(void)
{
    // モデルロード
    int modelId = -1;
    switch (type_)
    {
    case TYPE::type1: modelId = MV1LoadModel("Data/Model/Item/crystal1.mv1"); break;
    case TYPE::type2: modelId = MV1LoadModel("Data/Model/Item/crystal2.mv1"); break;
    default: break;
    }
    if (modelId != -1)
    {
        auto* mesh = AddComponent<StaticMeshComponent>(modelId);
        mesh->SetProfile(CollisionProfileType::ITEM);
    }

    // トランスフォーム
    SetScl(Vector3(50.0f, 50.0f, 50.0f));
    Vector3 upVec = defaultPos_.Normalized();
    Vector3 elevated = defaultPos_ + upVec * 34.0f;
    SetPos(elevated);
    Vector3 up = elevated.Normalized();
    Vector3 fwd = Vector3::FORWARD;
    if (fabsf(Vector3::Dot(fwd, up)) > 0.99f) fwd = Vector3::RIGHT;
    fwd = (fwd - up * Vector3::Dot(fwd, up)).Normalized();
    SetRot(Quaternion::LookRotation(fwd, up));

    // コライダー
    sphere_ = AddComponent<SphereComponent>(50.0f);
    sphere_->SetProfile(CollisionProfileType::ITEM);

    // Component を Init
    ActorBase::Init();

    // 状態管理のバインド
    stateChanges_[STATE::DROPPED] = std::bind(&Item::ChangeDropped, this);
    stateChanges_[STATE::HELD] = std::bind(&Item::ChangeHeld, this);
    stateChanges_[STATE::THROW] = std::bind(&Item::ChangeThrow, this);
    stateChanges_[STATE::DELIVERED] = std::bind(&Item::ChangeDelivered, this);
    ChangeState(STATE::DROPPED);
}

void Item::ChangeState(STATE state)
{
    state_ = state;
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

void Item::UpdateDropped(void) {}
void Item::UpdateHeld(void) {}
void Item::UpdateThrow(void) {}
void Item::UpdateDelivered(void) {}

void Item::DrawDropped(void)
{
    if (!isAimed_) return;
    VECTOR screenPos = ConvWorldPosToScreenPos(GetPos().ToVECTOR());
    if (screenPos.z <= 1.0f)
    {
        int prev = GetFontSize();
        SetFontSize(32);
        DrawFormatString((int)screenPos.x, (int)screenPos.y,
            GetColor(255, 255, 255), "$%d", value_);
        SetFontSize(prev);
    }
}
void Item::DrawHeld(void) {}
void Item::DrawThrow(void) {}
void Item::DrawDelelivered(void) {}
void Item::UpdateThrowMove(void) {}

void Item::DrawBillboard(void) const
{
    VECTOR vec = VNorm(VSub(camPos_.ToVECTOR(), GetPos().ToVECTOR()));
    VECTOR billPos = VAdd(GetPos().ToVECTOR(), VScale(vec, 32.0f));
    int idx = static_cast<int>(type_);
    if (idx >= 0 && idx < 3 && valueBillImg_[idx] != -1)
        DrawBillboard3D(billPos, 0.5f, 0.5f, 50.0f, 0.0f, valueBillImg_[idx], true);
}