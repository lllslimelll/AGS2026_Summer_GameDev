#pragma once
#include <array>
#include <vector>
#include <Dxlib.h>
#include "../Common/Vector2.h"

// 周辺機器種別
enum class PeripheralType
{
	KEYBOAD, // キーボード
	PAD,     // ジョイパッド
	MOUSE    // マウス
};

// 入力コマンドとデバイスの対応付け
struct InputBinding
{
	PeripheralType type; // 周辺機器種別
	unsigned int id;     // 実入力の値
};

// 入力命令コマンド
enum class InputCommand
{
	MOVE_FORWARD,// 前進
	MOVE_BACK,   // 後退
	MOVE_LEFT,   // 左移動
	MOVE_RIGHT,  // 右移動

	JUMP,	// ジャンプ
	BOOST,	// ブースト

	PICK_UP,  // 拾う
	PUT_DOWN, // 落とす
	DELIVER,  // 納品
	
	PAUSE,	// ポーズ
	
	UI_UP,	  // 上移動
	UI_DOWN,  // 下移動
	UI_LEFT,  // 左移動
	UI_RIGHT, // 右移動
	UI_DECIDE,// 決定
	UI_CANCEL,// キャンセル

	MAX
};

class InputManager
{

public:
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

	bool IsPressed(const std::string& name) const;
	bool IsTriggerd(const std::string& name) const;
	bool IsReleased(const std::string& name) const;

	// マウス座標の取得
	Vector2 GetMousePos(void) const;
	// マウスホイールの回転量を取得
	int GetMouseWheelRot(void) const;

	// 左スティックの方向を取得
	VECTOR GetLeftStickDirection(void) const;   
	// 右スティックの方向を取得
	VECTOR GetRightStickDirection(void) const; 

private:

	// 入力コマンドと入力バインディングのマッピングテーブル
	using InputTable_t = std::array<std::vector<InputBinding>, static_cast<int>(InputCommand::MAX)>;
	InputTable_t inputTable_;

	// 押されたかどうか記録用
	std::array<bool, static_cast<int>(InputCommand::MAX)> currentInputInfo_;
	std::array<bool, static_cast<int>(InputCommand::MAX)> lastInputInfo_;

	Vector2 mousePos_;	// 座標
	int mouseWheelRot_; // ホイール回転量

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	InputManager(void);

	// コピーコンストラクタも同様
	InputManager(const InputManager& instance) = default;

	// デストラクタも同様
	~InputManager(void) = default;

	// 内部処理
	void SetupBindings(void);
	bool EvaluateBinding(const InputBinding& binding) const;

};