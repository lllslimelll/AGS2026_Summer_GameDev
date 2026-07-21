#pragma once
#include <DxLib.h>
#include "../../Core/Vector3.h"
#include "../../Core/Quaternion.h"
#include "../../Utility/Math.h"
#include "../Actor/ActorBase.h"

class Player;
class Player;

class Camera : public ActorBase
{
public:

    // カメラモード
    enum class MODE
    {
        NONE,
        FIXED_POINT,  // 固定点カメラ
        FREE,         // 自由移動カメラ
        FOLLOW,       // プレイヤー追従カメラ
    };

    // ---- 定数 ----

    // 初期座標
    static constexpr Vector3 DEFAULT_POS = Vector3(0.0f, 200.0f, -500.0f);
    // 初期ピッチ角（ラジアン、やや下向き）
    static constexpr float   DEFAULT_PITCH_ANGLE = Math::ToRadian(10.0f);

    // 移動速度（FREE モード）
    static constexpr float SPEED = 20.0f;

    // クリップ距離
    static constexpr float VIEW_NEAR = 3.0f;
    static constexpr float VIEW_FAR = 20000.0f;

    // 追従カメラのオフセット（プレイヤー位置基準、後ろ・上にずらす）
    static constexpr Vector3 FOLLOW_CAMERA_OFFSET = Vector3(0.0f, 150.0f, -350.0f);
    // 注視点のプレイヤーからの高さオフセット
    static constexpr float   FOLLOW_TARGET_HEIGHT = 100.0f;

    // 上下回転制限（ラジアン）
    static constexpr float LIMIT_PITCH_MAX = Math::ToRadian(60.0f);
    static constexpr float LIMIT_PITCH_MIN = Math::ToRadian(-70.0f);

    // スティック回転速度
    static constexpr float ROT_POW_RAD = Math::ToRadian(2.0f);

    // マウス感度
    static constexpr float MOUSE_SENSITIVITY = 0.00040f;

    // コンストラクタ / デストラクタ
    Camera(void);
    ~Camera(void) override;

    // ライフサイクル
    void Init(void)    override;
    void Update(void)  override;
    void Draw(void)    override;
    void Release(void) override;

    // 描画前に DxLib カメラを設定する
    void SetBeforeDraw(void);

    // 追従対象を設定する（毎フレーム SetCameraForward も呼ぶ）
    void SetFollowTarget(Player* target);

    // カメラモードを変更する
    void ChangeMode(MODE mode);

    // 注視点を取得する
    Vector3 GetTargetPos(void) const;

    // カメラ前方向を取得する（注視点 - 位置）
    Vector3 GetCameraForward(void) const;

private:

    // マウス初回フレームフラグ
    bool isFirstMouseFrame_ = true;

    // 追従対象
    Player* followTarget_ = nullptr;

    // カメラモード
    MODE mode_ = MODE::NONE;

    // 水平回転角（ヨー）・垂直回転角（ピッチ）（ラジアン）
    float yawAngle_ = 0.0f;
    float pitchAngle_ = DEFAULT_PITCH_ANGLE;

    // 注視点（DxLib に渡す用）
    Vector3 targetPos_ = Vector3::ZERO;

    // モード別更新
    void UpdateFixedPoint(void);
    void UpdateFree(void);
    void UpdateFollow(void);

    // 初期値にリセット
    void SetDefault(void);

    // 回転入力処理
    void ProcessRot(bool isLimit);
    void RotMouse(bool isLimit);
    void RotGamePad(bool isLimit);

    // 自由移動処理（FREE モード用）
    void ProcessMove(void);
};