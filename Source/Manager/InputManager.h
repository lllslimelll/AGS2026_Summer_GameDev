#pragma once
#include <unordered_map>
#include <vector>
#include <Dxlib.h>
#include "../Core/Vector2.h"

class InputManager
{

public:

	// 入力命令コマンド
	enum class InputCommand
	{
		// 移動用
		MOVE_FORWARD,// 前進
		MOVE_BACK,   // 後退
		MOVE_LEFT,   // 左移動
		MOVE_RIGHT,  // 右移動

		// アクション用
		JUMP,	  // ジャンプ
		BOOST, 	  // ブースト
		PICK_UP,  // 拾う
		DROP,	  // 落とす
		DELIVERY, // 納品
		RETURN,	  // 帰還

		// インベントリ用
		SLOT_1,     // スロット１
		SLOT_2,     // スロット２
		SLOT_3,     // スロット３
		SLOT_4,     // スロット４
		SLOT_5,     // スロット５
		SLOT_LEFT,  // スロット左へ
		SLOT_RIGHT, // スロット右へ

		// システム用
		PAUSE,		// ポーズ

		// UI用
		UI_UP,	  // 上移動
		UI_DOWN,  // 下移動
		UI_LEFT,  // 左移動
		UI_RIGHT, // 右移動
		UI_DECIDE,// 決定
		UI_CANCEL,// キャンセル

		// デバッグ用
		SET_POINT,	  // 座標ポイント設定
		DELETE_POINT, // 座標ポイント削除
		SAVE_POINT,	  // 座標ポイント保存

		MAX
	};

	// インスタンスを明示的に生成
	static void CreateInstance(void);
	// インスタンスの取得
	static InputManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// リソースの破棄
	void Destroy(void);

	// 入力状態の取得
	// 押されているか
	bool IsPressed(InputCommand cmd) const;
	// 押されたか
	bool IsTriggered(InputCommand cmd) const;
	// 離されたか
	bool IsReleased(InputCommand cmd) const;

	// マウスホイールの回転量を取得
	int GetMouseWheelRot(void) const;

	// 左スティックの方向を取得（移動用）
	VECTOR GetLeftStickDirection(void) const;   
	// 右スティックの方向を取得（視点用）
	VECTOR GetRightStickDirection(void) const; 

private:
	
	// 周辺機器種別
	enum class PeripheralType
	{
		KEYBOARD, // キーボード
		PAD,     // パッド
		MOUSE    // マウス
	};

	// 入力状態
	struct InputState
	{
		PeripheralType type; // 周辺機器種別
		unsigned int id;     // 実入力の値
	};


	// コントローラーの認識番号
	enum class PAD_NO
	{
		KEY_PAD1,        // キー入力とパッド１入力
		PAD1,            // パッド１入力
		PAD2,            // パッド２入力
		PAD3,            // パッド３入力
		PAD4,            // パッド４入力
		INPUT_KEY = 4096 // キー入力
	};

	// コントローラーの種別
	enum class PAD_TYPE
	{
		OTHER = 0,
		XBOX_360,
		XBOX_ONE,
		DUAL_SHOCK_4,
		DUAL_SENSE,
		MAX
	};

	// コントローラーのボタン
	enum class PAD_BUTTON
	{
		// フェイスボタン
		TOP,	 // Xbox:Y / PS4:△
		BOTTOM,  // Xbox:A / PS4:×
		LEFT,    // Xbox:X / PS4:□
		RIGHT,   // Xbox:B / PS4:〇

		// ショルダー
		L_SHOULDER,   // Xbox:LB / PS4:L1
		R_SHOULDER,   // Xbox:RB / PS4:R1
		L_TRIGGER,    // Xbox:LT / PS4:L2
		R_TRIGGER,    // Xbox:RT / PS4:R2

		// スティック
		L_THUMB,      // 左スティック押し込み
		R_THUMB,      // 右スティック押し込み

		// 方向キー
		D_UP,    // 上
		D_DOWN,  // 下
		D_LEFT,  // 左
		D_RIGHT, // 右

		// スタート
		START,
	};

	// スティックの入力情報
	struct STICK_STATE
	{
		float leftX;  // 左スティックX軸
		float leftY;  // 左スティックY軸
		float rightX; // 右スティックX軸
		float rightY; // 右スティックY軸
	};


	// シングルトン用静的インスタンス
	static InputManager* instance_;

	// 入力コマンドと入力デバイスの対応付けテーブル
	using InputTable_t = std::unordered_map<InputCommand, std::vector<InputState>>;
	InputTable_t inputTable_;

	// 押されたかどうか記録用
	std::unordered_map<InputCommand, bool> currentInputInfo_;
	std::unordered_map<InputCommand, bool> lastInputInfo_;

	int mouseWheelRot_; // ホイール回転量

	// スティック状態
	STICK_STATE stickState_;

	// アナログキーの入力受付しきい値(0.0～1.0)
	static constexpr float THRESHOLD = 0.35f;

	// コントローラー情報
	DINPUT_JOYSTATE dInState_;
	XINPUT_STATE    xInState_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	InputManager(void);
	// コピーコンストラクタも同様
	InputManager(const InputManager& instance) = default;
	// デストラクタも同様
	~InputManager(void) = default;

	// 入力コマンドとデバイスの対応付けを設定
	void SetUpBindings(void);

	// コントローラーのボタンが押されているか判定する
	bool IsPadButtonActive(PAD_BUTTON button, PAD_TYPE type,
		const XINPUT_STATE& xState, const DINPUT_JOYSTATE& dState) const;

	// アナログキーの入力値から方向を取得
	VECTOR GetDirectionXZAKey(float aKeyX, float aKeyY) const;

	// コントローラーの種別を取得する
	PAD_TYPE GetPadType(PAD_NO padNo) const;

	// DirectInput形式でコントローラーの入力情報を取得する
	DINPUT_JOYSTATE GetPadDInputState(PAD_NO padNo);

	// XInput形式でコントローラーの入力情報を取得する（XBOX用）
	XINPUT_STATE GetPadXInputState(PAD_NO padNo);

	// コントローラーの入力情報を取得してStickStateに変換する
	STICK_STATE GetStickState(PAD_NO padNo);

	// スティック状態を更新する
	void UpdateStickState(PAD_NO padNo);
};