#pragma once
#include <DxLib.h>
class Player;

class Camera
{
public:

	// カメラの初期位置
	static constexpr VECTOR DEFAULT_POS = { 0.0f, 1000.0f, -1600.0f };

	// カメラの初期角度
	static constexpr VECTOR DEFAULT_ANGLES = { 30.0f * DX_PI_F / 180.0f, 0.0f, 0.0f };

	// カメラクリップ：NEAR
	static constexpr float CAMERA_NEAR = 3.0f;

	// カメラクリップ：FAR
	static constexpr float CAMERA_FAR = 5000.0f;

	// カメラスピード（移動）
	static constexpr float SPEED_MOVE = 10.0f;

	// カメラスピード（度）
	static constexpr float SPEED_ANGLE_DEG = 1.0f;

	Camera(void);
	~Camera(void);

	// 初期化
	void Init(void);
	// 更新
	void Update(void);
	
	// 描画前のカメラ設定
	void SetBeforeDraw(void);
	// デバック描画
	void DrawDebug(void);

	// 解放
	void Release(void);

	// 追従対象の設定
	void SetFollow(Player* player);

private:

	Player* player_;

	// カメラの位置
	VECTOR pos_;

	// カメラ角度
	VECTOR angles_;
};