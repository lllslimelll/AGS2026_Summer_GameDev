#include "../../../Common/AnimationController.h"
#include "../../../Utility/Math.h"
#include "../../Scene/SceneManager.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SoundManager.h"
#include "../../../Component/CapsuleComponent.h"
#include "../../../Component/StaticMeshComponent.h"
#include "../../../Collision/CollisionManager.h"
#include "../../../Collision/CollisionQueryParams.h"
#include "../Item/ItemManager.h"
#include "../Stage/StageManager.h"
#include "../Stage/Rocket.h"
#include "../Item/Item.h"
#include "Player.h"

Player::Player(ItemManager* itemMng, StageManager& stageMng)
    : CharactorBase()
    , stageMng_(stageMng)
    , itemMgr_(itemMng)
{
}

Player::~Player(void)
{
}

void Player::Init(void)
{
    // ---- モデルロード ----
    int modelId = resMng_.Load(ResourceManager::SRC::PLAYER).handleId_;
    auto* mesh = AddComponent<StaticMeshComponent>(modelId);
    mesh->SetProfile(CollisionProfileType::PAWN);

    // ---- アニメーション ----
    animCtrl_ = new AnimationController(modelId);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::IDLE), 20.0f, resMng_.Load(ResourceManager::SRC::IDLE).path_);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::RUN), 20.0f, resMng_.Load(ResourceManager::SRC::RUN).path_);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN), 20.0f, resMng_.Load(ResourceManager::SRC::FAST_RUN).path_);
    animCtrl_->Add(static_cast<int>(ANIM_TYPE::JUMP), 20.0f, resMng_.Load(ResourceManager::SRC::JUMP_RISING).path_);

    // ---- トランスフォーム ----
    SetScl(Vector3::ONE);
    SetRot(Quaternion::AngleAxis(Math::ToRadian(180.0f), Vector3::UP));
    SetPos(Vector3(1386.0f, -910.0f, -94.8f));

    // ---- カプセルコライダー ----
    constexpr float centerY = (COL_CAPSULE_TOP_LOCAL_POS.y + COL_CAPSULE_DOWN_LOCAL_POS.y) * 0.5f;
    constexpr float halfHeight = (COL_CAPSULE_TOP_LOCAL_POS.y - COL_CAPSULE_DOWN_LOCAL_POS.y) * 0.5f;
    capsule_ = AddComponent<CapsuleComponent>(COL_CAPSULE_RADIUS, halfHeight);
    capsule_->SetProfile(CollisionProfileType::PAWN);
    capsule_->SetLocalPos(Vector3(0.0f, centerY, 0.0f));

    // ---- Component の Init ----
    ActorBase::Init();

    // ---- 後処理 ----
    animCtrl_->Play(0, true);

    stateChanges_.emplace(static_cast<int>(STATE::IDLE), std::bind(&Player::ChangeStateIdle, this));
    stateChanges_.emplace(static_cast<int>(STATE::DEAD), std::bind(&Player::ChangeStateDead, this));
    stateChanges_.emplace(static_cast<int>(STATE::END), std::bind(&Player::ChangeStateEnd, this));
    ChangeState(STATE::IDLE);

    // 初期向きを XZ 平面上に整列させる
    // XY 平面化後は上方向が Vector3::UP 固定
    Vector3 up = Vector3::UP;
    float   d = Vector3::Dot(faceDir_, up);
    Vector3 f = faceDir_ - up * d;
    if (f.IsNearlyZero()) f = Vector3::FORWARD;
    faceDir_ = f.Normalized();
    SetRot(Quaternion::LookRotation(faceDir_, up));
}

// ---- ライフサイクル ----

void Player::Draw(void)
{
    int screenW, screenH;
    GetScreenState(&screenW, &screenH, nullptr);

    // 照準サークルを画面中央に描画
    int  cx = screenW / 2;
    int  cy = screenH / 2;
    int  radius = static_cast<int>(crosshairRadius_);
    bool isFilled = (crosshairRadius_ <= 7.0f);
    DrawCircle(cx, cy, radius, GetColor(255, 255, 255), isFilled ? TRUE : FALSE);
}

// ---- 状態遷移 ----

void Player::ChangeState(STATE state)
{
    state_ = state;
    CharactorBase::ChangeState(static_cast<int>(state_));
}

void Player::ChangeStateIdle(void)
{
    stateUpdate_ = std::bind(&Player::UpdateIdle, this);
}

void Player::ChangeStateDead(void)
{
    stateUpdate_ = std::bind(&Player::UpdateDead, this);
    animCtrl_->Play(static_cast<int>(ANIM_TYPE::DEAD), false);
    stateUpdate_ = []() {};

    // 死亡オーバーレイを表示
    SceneManager::GetInstance().PushOverlay(SceneManager::SCENE_ID::DEAD);
    SoundManager::GetInstance().StopWalk();
}

void Player::ChangeStateEnd(void)
{
    // END 状態では何もしない
}

// ---- 更新 ----

void Player::UpdateProcess(void)
{
    stateUpdate_();
}

void Player::UpdateProcessPost(void)
{
    if (state_ == STATE::DEAD || state_ == STATE::END) return;

    UpdateItem();
    ChangeSelectedSlot();
}

void Player::UpdateIdle(void)
{
    ProcessMove();
    ProcessJump();
    CollisionReserve();
    UpdateOxygenAndHp();
}

void Player::UpdateDead(void)
{
    // 死亡アニメーション終了後に END 状態へ
    if (animCtrl_->IsEnd()) ChangeState(STATE::END);
}

// ---- 移動 ----

void Player::ProcessMove(void)
{
    auto& ins = InputManager::GetInstance();

    movePow_ = Vector3::ZERO;

    // キーボード入力方向（ローカル座標系）
    Vector3 kDir = Vector3::ZERO;
    if (ins.IsPressed(InputManager::InputCommand::MOVE_FORWARD)) kDir = kDir + Vector3::FORWARD;
    if (ins.IsPressed(InputManager::InputCommand::MOVE_BACK))    kDir = kDir + Vector3::BACK;
    if (ins.IsPressed(InputManager::InputCommand::MOVE_LEFT))    kDir = kDir + Vector3::LEFT;
    if (ins.IsPressed(InputManager::InputCommand::MOVE_RIGHT))   kDir = kDir + Vector3::RIGHT;
    if (!kDir.IsNearlyZero()) kDir = kDir.Normalized();

    // スティック入力
    VECTOR stick = ins.GetInstance().GetLeftStickDirection();
    Vector3 dDir = Vector3(stick.x, 0.0f, stick.z);

    // ブースト入力
    if (ins.IsTriggered(InputManager::InputCommand::BOOST)) isBoost_ = true;

    // ジャンプ中は歩行 SE を止める
    if (isJump_) SoundManager::GetInstance().StopWalk();

    bool hasInput = !kDir.IsNearlyZero() || !dDir.IsNearlyZero();

    if (hasInput)
    {
        SoundManager::GetInstance().PlayWalk();

        // XY 平面化後は上方向が Y 固定
        // カメラ前方向をもとに水平面上の前方向・右方向を計算
        Vector3 forward = Vector3(cameraForward_.x, 0.0f, cameraForward_.z).Normalized();
        Vector3 right = Vector3::Cross(Vector3::UP, forward).Normalized();

        // プレイヤーはカメラの向きを向く
        faceDir_ = forward;

        // 入力ベクトルをワールド空間に変換して合成
        Vector3 moveVec =
            forward * (kDir.z + dDir.z) +
            right * (kDir.x + dDir.x);

        moveDir_ = moveVec.IsNearlyZero() ? moveDir_ : moveVec.Normalized();

        if (isBoost_)
        {
            moveSpeed_ = SPEED_DASH;
            animCtrl_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN), true);
            SoundManager::GetInstance().StopWalk();
            SoundManager::GetInstance().PlayBoost();
        }
        else
        {
            moveSpeed_ = SPEED_MOVE;
            animCtrl_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
        }

        movePow_ = moveDir_ * moveSpeed_;
    }
    else
    {
        SoundManager::GetInstance().StopWalk();
        SoundManager::GetInstance().StopBoost();

        // 静止中もカメラ前方を向き続ける
        faceDir_ = Vector3(cameraForward_.x, 0.0f, cameraForward_.z).Normalized();

        if (!isJump_) animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);

        isBoost_ = false;
    }

    // 後退中はブーストを無効化
    if (kDir.z < 0.0f || dDir.z < 0.0f) isBoost_ = false;
}

void Player::ProcessJump(void)
{
    auto& ins = InputManager::GetInstance();

    // XY 平面化後は上方向が Y 固定
    Vector3 upDir = Vector3::UP;

    bool triggered = ins.IsTriggered(InputManager::InputCommand::JUMP);
    bool pressed = ins.IsPressed(InputManager::InputCommand::JUMP);

    // 初期ジャンプ
    if (triggered && !isJump_)
    {
        jumpPow_ = upDir * (POW_JUMP_INIT * scnMng_.GetDeltaTime());
        stepJump_ = 0.0f;
        isJump_ = true;
        animCtrl_->Play(static_cast<int>(ANIM_TYPE::JUMP), false);
    }

    // 長押しによる持続ジャンプ
    if (pressed)
    {
        stepJump_ += scnMng_.GetDeltaTime();
        if (stepJump_ < TIME_JUMP_INPUT)
        {
            jumpPow_ = jumpPow_ + upDir * (POW_JUMP_KEEP * scnMng_.GetDeltaTime());
        }
    }
    else
    {
        // 離したら持続ジャンプを打ち切る
        stepJump_ = TIME_JUMP_INPUT;
    }
}

// ---- コライダー調整 ----

void Player::CollisionReserve(void)
{
    if (capsule_ == nullptr) return;

    // ジャンプ中はカプセルを伸ばす
    if (animCtrl_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP))
    {
        constexpr float cy = (COL_CAPSULE_TOP_JUMP_LOCAL_POS.y + COL_CAPSULE_DOWN_JUMP_LOCAL_POS.y) * 0.5f;
        constexpr float half = (COL_CAPSULE_TOP_JUMP_LOCAL_POS.y - COL_CAPSULE_DOWN_JUMP_LOCAL_POS.y) * 0.5f;
        capsule_->SetLocalPos(Vector3(0.0f, cy, 0.0f));
        capsule_->SetHalfHeight(half);
    }
    else
    {
        constexpr float cy = (COL_CAPSULE_TOP_LOCAL_POS.y + COL_CAPSULE_DOWN_LOCAL_POS.y) * 0.5f;
        constexpr float half = (COL_CAPSULE_TOP_LOCAL_POS.y - COL_CAPSULE_DOWN_LOCAL_POS.y) * 0.5f;
        capsule_->SetLocalPos(Vector3(0.0f, cy, 0.0f));
        capsule_->SetHalfHeight(half);
    }
}

// ---- アイテム ----

void Player::UpdateItem(void)
{
    UpdateAimedItem();
    ProcessPickUp();
    ProcessDelivery();
    ProcessThrow();
    ProcessDrop();

    // 照準サークルの半径をスムーズに補間
    float targetRadius = (CanPickUp() || IsAimingRocket()) ? 20.0f : 5.0f;
    crosshairRadius_ += (targetRadius - crosshairRadius_) * 15.0f * scnMng_.GetDeltaTime();

    // ロケットに照準が当たっている状態で帰還ボタンを押したらリザルトへ
    if (IsAimingRocket())
    {
        auto& ins = InputManager::GetInstance();
        if (ins.IsTriggered(InputManager::InputCommand::RETURN))
        {
            SceneManager::GetInstance().SetResultScore(stageMng_.GetRocket().GetTotalDelivered());
            SceneManager::GetInstance().PushOverlay(SceneManager::SCENE_ID::RESULT);
        }
    }
}

void Player::UpdateAimedItem(void)
{
    // カメラ前方向でレイを飛ばして照準中のアイテムを取得
    Item* newAimed = itemMgr_->GetAimedItem(
        GetPos(),
        cameraForward_,
        Item::RANGE_PICKUP);

    if (newAimed == aimedItem_) return;

    if (aimedItem_ != nullptr) aimedItem_->SetAimed(false);
    if (newAimed != nullptr) newAimed->SetAimed(true);
    aimedItem_ = newAimed;
}

void Player::UpdateFollowItem(void)
{
    // TODO: モデルフレームからアイテム保持位置を取得する
}

void Player::ProcessPickUp(void)
{
    if (aimedItem_ == nullptr ||
        aimedItem_->GetState() != Item::STATE::DROPPED) return;

    auto& ins = InputManager::GetInstance();
    if (!ins.IsTriggered(InputManager::InputCommand::PICK_UP)) return;

    if (CanPickUp())
    {
        aimedItem_->OnPickedUp();
        inventory_.Add(aimedItem_);
    }
}

void Player::ProcessThrow(void)
{
    // TODO: 投擲実装
}

void Player::ProcessDrop(void)
{
    auto& ins = InputManager::GetInstance();
    if (!ins.IsTriggered(InputManager::InputCommand::DROP)) return;

    Item* item = inventory_.GetSelected();
    if (item == nullptr) return;

    item->OnDrop(GetPos());
    inventory_.RemoveSelected();
}

void Player::ProcessDelivery(void)
{
    // アイテムが照準に入っている間はピックアップを優先
    if (aimedItem_ != nullptr) return;
    if (!IsAimingRocket()) return;

    Item* item = inventory_.GetSelected();
    if (item == nullptr) return;

    auto& ins = InputManager::GetInstance();
    if (!ins.IsPressed(InputManager::InputCommand::PICK_UP)) return;

    stageMng_.GetRocket().AddDelivery(item->GetValue());
    item->OnDelivered();
    inventory_.RemoveSelected();
}

// ---- ゲッター / セッター ----

void Player::SetCameraForward(const Vector3& forward)
{
    // XZ 平面に投影して正規化する（Y 成分を捨てる）
    cameraForward_ = Vector3(forward.x, 0.0f, forward.z).Normalized();
}

int   Player::GetHp(void)     const { return hp_; }
float Player::GetOxygen(void) const { return oxygen_; }

const Inventory& Player::GetInventory(void) const { return inventory_; }

Player::GUIDE_INFO Player::GetGuideInfo(void) const
{
    GUIDE_INFO info;
    info.canPickUp = CanPickUp();
    info.isAimingRocket = IsAimingRocket();
    info.hasSelectedItem = inventory_.GetSelected() != nullptr;
    info.isIdle = state_ == STATE::IDLE;
    info.isPad = GetJoypadNum() != 0;
    info.hasAnyItem = inventory_.HasAnyItem();
    info.totalDelivered = stageMng_.GetRocket().GetTotalDelivered();
    info.quota = Rocket::QUOTA;
    return info;
}

// ---- ユーティリティ ----

bool Player::CanPickUp(void) const
{
    if (aimedItem_ == nullptr ||
        aimedItem_->GetState() != Item::STATE::DROPPED) return false;
    return !inventory_.IsFull();
}

bool Player::IsAimingRocket(void) const
{
    // カメラ前方向にレイを飛ばして Rocket に当たるか判定
    Vector3 origin = GetPos();
    Vector3 end = origin + cameraForward_ * Item::RANGE_PICKUP;

    HitResult hit = CollisionManager::GetInstance().LineTrace(
        origin.ToVECTOR(),
        end.ToVECTOR(),
        CollisionChannel::WORLD_STATIC,
        CollisionQueryParams());

    if (!hit.isHit) return false;

    return dynamic_cast<Rocket*>(hit.otherActor) != nullptr;
}

void Player::ChangeSelectedSlot(void)
{
    auto& ins = InputManager::GetInstance();

    if (ins.IsTriggered(InputManager::InputCommand::SLOT_LEFT))  inventory_.SelectPrev();
    if (ins.IsTriggered(InputManager::InputCommand::SLOT_RIGHT)) inventory_.SelectNext();
    if (ins.IsTriggered(InputManager::InputCommand::SLOT_1))     inventory_.Select(0);
    if (ins.IsTriggered(InputManager::InputCommand::SLOT_2))     inventory_.Select(1);
    if (ins.IsTriggered(InputManager::InputCommand::SLOT_3))     inventory_.Select(2);
    if (ins.IsTriggered(InputManager::InputCommand::SLOT_4))     inventory_.Select(3);
    if (ins.IsTriggered(InputManager::InputCommand::SLOT_5))     inventory_.Select(4);

    int wheel = ins.GetMouseWheelRot();
    if (wheel > 0) inventory_.SelectPrev();
    if (wheel < 0) inventory_.SelectNext();
}

void Player::DrawFrameRecursive(int modelId, int frameIdx)
{
    MV1DrawFrame(modelId, frameIdx);
    int childNum = MV1GetFrameChildNum(modelId, frameIdx);
    for (int i = 0; i < childNum; i++)
    {
        DrawFrameRecursive(modelId, MV1GetFrameChild(modelId, frameIdx, i));
    }
}

void Player::UpdateOxygenAndHp(void)
{
    if (state_ == STATE::DEAD || state_ == STATE::END) return;

    const float dt = scnMng_.GetDeltaTime();

    // ブースト中は酸素消費が 2 倍
    float consumeRate = isBoost_ ? OXYGEN_DASH_RATE : 1.0f;
    oxygen_ = Math::Clamp(oxygen_ - dt * consumeRate, 0.0f, MAX_OXYGEN);

    if (oxygen_ > 0.0f)
    {
        suffocateTimer_ = 0.0f;
        return;
    }

    // 酸素切れ：一定間隔でダメージ
    suffocateTimer_ += dt;
    if (suffocateTimer_ >= SUFFOCATE_INTERVAL)
    {
        suffocateTimer_ -= SUFFOCATE_INTERVAL;
        OnDamaged(SUFFOCATE_DAMAGE);
    }
}

void Player::OnDamaged(int damage)
{
    hp_ -= damage;
    SoundManager::GetInstance().PlayDamaged();

    if (hp_ <= 0)
    {
        hp_ = 0;
        ChangeState(STATE::DEAD);
    }
}