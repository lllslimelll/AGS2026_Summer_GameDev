#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../../Utility/Math.h"
#include "../../Manager/InputManager.h"
#include "../Actor/Charactor/Player.h"
#include "Camera.h"

Camera::Camera(void)
    : ActorBase()
    , mode_(MODE::NONE)
    , yawAngle_(0.0f)
    , pitchAngle_(DEFAULT_PITCH_ANGLE)
{
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
    SetDefault();
    ActorBase::Init();
}

void Camera::Update(void)
{
    switch (mode_)
    {
    case MODE::FIXED_POINT: UpdateFixedPoint(); break;
    case MODE::FREE:        UpdateFree();        break;
    case MODE::FOLLOW:      UpdateFollow();      break;
    default: break;
    }

    ActorBase::Update();
}

void Camera::Draw(void)
{
    // カメラ自身は描画しない
}

void Camera::Release(void)
{
    ActorBase::Release();
}

void Camera::SetBeforeDraw(void)
{
    // クリップ距離を設定（SetDrawScreen でリセットされるため毎フレーム呼ぶ）
    SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

    // DxLib にカメラ位置・注視点・上方向を設定
    SetCameraPositionAndTargetAndUpVec(
        GetPos().ToVECTOR(),
        targetPos_.ToVECTOR(),
        Vector3::UP.ToVECTOR()
    );

    // Effekseer とカメラを同期
    Effekseer_Sync3DSetting();
}

void Camera::SetFollowTarget(Player* player)
{
    followTarget_ = player;
}

void Camera::ChangeMode(MODE mode)
{
    SetDefault();
    mode_ = mode;

    if (mode_ == MODE::FOLLOW)
    {
        // マウスカーソルを画面中央にリセット
        int sw, sh;
        GetDrawScreenSize(&sw, &sh);
        SetMousePoint(sw / 2, sh / 2);
    }
}

Vector3 Camera::GetTargetPos(void) const
{
    return targetPos_;
}

Vector3 Camera::GetCameraForward(void) const
{
    return (targetPos_ - GetPos()).Normalized();
}

// ---- モード別更新 ----

void Camera::UpdateFixedPoint(void)
{
    // 固定点カメラは何もしない
}

void Camera::UpdateFree(void)
{
    ProcessRot(false);
    ProcessMove();

    // yaw + pitch から回転を作り注視点を計算
    Quaternion rot = Quaternion::AngleAxis(yawAngle_, Vector3::UP)
        * Quaternion::AngleAxis(pitchAngle_, Vector3::RIGHT);

    targetPos_ = GetPos() + rot.RotateVector(Vector3::FORWARD * 100.0f);
}

void Camera::UpdateFollow(void)
{
    if (followTarget_ == nullptr) return;

    // 注視点までの距離（位置と注視点が一致すると前方向が
    // ゼロベクトルになり壊れるため、必ず一定距離離す）
    constexpr float LOOK_DIST = 100.0f;

    // 回転入力を受け取る
    ProcessRot(true);

    // ---- 一人称視点カメラ ----
    // yaw（水平）と pitch（垂直）から「視線方向」を直接作る。
    Quaternion rot = Quaternion::AngleAxis(yawAngle_, Vector3::UP)
        * Quaternion::AngleAxis(pitchAngle_, Vector3::RIGHT);

    // カメラ位置 = プレイヤーの目の高さ（オフセットは回転させない）
    SetPos(followTarget_->GetPos() + FOLLOW_CAMERA_OFFSET);

    // 注視点 = カメラ位置から視線方向へ一定距離延ばした点
    targetPos_ = GetPos() + rot.RotateVector(Vector3::FORWARD) * LOOK_DIST;

    // プレイヤーにカメラ前方向を直接注入する
    followTarget_->SetCameraForward(GetCameraForward());

    // 照準・インタラクト用にカメラのレイ（位置と生の向き）も注入する
    followTarget_->SetCameraRay(GetPos(), GetCameraForward());
}

// ---- 内部処理 ----

void Camera::SetDefault(void)
{
    SetPos(DEFAULT_POS);
    targetPos_ = Vector3::ZERO;
    pitchAngle_ = DEFAULT_PITCH_ANGLE;
    yawAngle_ = 0.0f;
}

void Camera::ProcessRot(bool isLimit)
{
    RotMouse(isLimit);
    RotGamePad(isLimit);
}

void Camera::RotMouse(bool isLimit)
{
    int screenW, screenH;
    GetDrawScreenSize(&screenW, &screenH);
    const int centerX = screenW / 2;
    const int centerY = screenH / 2;

    // 初回はカーソルを中央に置くだけ
    if (isFirstMouseFrame_)
    {
        SetMousePoint(centerX, centerY);
        isFirstMouseFrame_ = false;
        return;
    }

    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);

    yawAngle_ += (mouseX - centerX) * MOUSE_SENSITIVITY;
    pitchAngle_ += (mouseY - centerY) * MOUSE_SENSITIVITY;

    if (isLimit)
    {
        pitchAngle_ = Math::Clamp(pitchAngle_, LIMIT_PITCH_MIN, LIMIT_PITCH_MAX);
    }

    // カーソルを中央に戻す
    SetMousePoint(centerX, centerY);
}

void Camera::RotGamePad(bool isLimit)
{
    auto& ins = InputManager::GetInstance();
    VECTOR dir = ins.GetInstance().GetRightStickDirection();

    yawAngle_ += dir.x * ROT_POW_RAD;
    pitchAngle_ -= dir.z * ROT_POW_RAD;

    if (isLimit)
    {
        pitchAngle_ = Math::Clamp(pitchAngle_, LIMIT_PITCH_MIN, LIMIT_PITCH_MAX);
    }
}

void Camera::ProcessMove(void)
{
    auto& ins = InputManager::GetInstance();

    Vector3 moveDir = Vector3::ZERO;

    if (GetJoypadNum() == 0)
    {
        if (ins.IsPressed(InputManager::InputCommand::MOVE_FORWARD)) moveDir = moveDir + Vector3::FORWARD;
        if (ins.IsPressed(InputManager::InputCommand::MOVE_BACK))    moveDir = moveDir + Vector3::BACK;
        if (ins.IsPressed(InputManager::InputCommand::MOVE_LEFT))    moveDir = moveDir + Vector3::LEFT;
        if (ins.IsPressed(InputManager::InputCommand::MOVE_RIGHT))   moveDir = moveDir + Vector3::RIGHT;
    }
    else
    {
        VECTOR stick = ins.GetInstance().GetLeftStickDirection();
        moveDir = Vector3(stick.x, 0.0f, stick.z);
    }

    if (moveDir.IsNearlyZero()) return;

    // yaw 回転だけ適用（上下移動は含めない）
    Quaternion rot = Quaternion::AngleAxis(yawAngle_, Vector3::UP);
    Vector3 worldDir = rot.RotateVector(moveDir.Normalized());

    SetPos(GetPos() + worldDir * SPEED);
    targetPos_ = targetPos_ + worldDir * SPEED;
}