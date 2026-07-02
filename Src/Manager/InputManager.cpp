#include <string>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "InputManager.h"

InputManager* InputManager::instance_ = nullptr;

void InputManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new InputManager();
	}
	instance_->Init();
}

InputManager& InputManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		InputManager::CreateInstance();
	}
	return *instance_;
}

void InputManager::Init(void)
{
	mouseWheelRot_ = 0;

	inputTable_["Up"] = { //{PeripheralType::KEYBOAD, KEY_INPUT_UP},
							//{PeripheralType::KEYBOAD, KEY_INPUT_W},
							{PeripheralType::PAD, XINPUT_BUTTON_DPAD_UP} };

	inputTable_["Down"] = { //{PeripheralType::KEYBOAD, KEY_INPUT_DOWN},
								//{PeripheralType::KEYBOAD, KEY_INPUT_S},
								{PeripheralType::PAD, PAD_INPUT_DOWN} };

	inputTable_["left"] = { {PeripheralType::KEYBOAD, KEY_INPUT_LEFT},
								{PeripheralType::KEYBOAD, KEY_INPUT_A},
								{PeripheralType::PAD, PAD_INPUT_LEFT} };

	inputTable_["right"] = { {PeripheralType::KEYBOAD, KEY_INPUT_RIGHT},
								{PeripheralType::KEYBOAD, KEY_INPUT_D},
								{PeripheralType::PAD, PAD_INPUT_RIGHT} };

	// DXライブラリはPADのボタン番号が6ボタンベースなのでずれています
	inputTable_["ok"] = { {PeripheralType::KEYBOAD, KEY_INPUT_RETURN},
								{PeripheralType::MOUSE, MOUSE_INPUT_LEFT},
								{PeripheralType::PAD, PAD_INPUT_DOWN} }; // SELECTボタン

	inputTable_["pause"] = { {PeripheralType::KEYBOAD, KEY_INPUT_ESCAPE},
								{PeripheralType::PAD, PAD_INPUT_R} }; // STARTボタン

	inputTable_["jump"] = { {PeripheralType::KEYBOAD, KEY_INPUT_Z},
								{PeripheralType::PAD, PAD_INPUT_C} }; // Xボタン

	inputTable_["attack"] = { {PeripheralType::KEYBOAD, KEY_INPUT_X},
								{PeripheralType::PAD, PAD_INPUT_A} }; // Aボタン

	for (auto& inputInfo : inputTable_)
	{
		currentInputInfo_[inputInfo.first] = false;
		lastInputInfo_[inputInfo.first] = false;
	}
}

void InputManager::Update(void)
{

	// キーボード検知
	for (auto& p : keyInfos_)
	{
		p.second.keyOld = p.second.keyNew;
		p.second.keyNew = CheckHitKey(p.second.key);
		p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
		p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
	}

	// マウス検知
	mouseInput_ = GetMouseInput();
	GetMousePoint(&mousePos_.x, &mousePos_.y);

	for (auto& p : mouseInfos_)
	{
		p.second.keyOld = p.second.keyNew;
		p.second.keyNew = mouseInput_ == p.second.key;
		p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
		p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
	}

	// パッド情報
	SetJPadInState(JOYPAD_NO::KEY_PAD1);
	SetJPadInState(JOYPAD_NO::PAD1);
	SetJPadInState(JOYPAD_NO::PAD2);
	SetJPadInState(JOYPAD_NO::PAD3);
	SetJPadInState(JOYPAD_NO::PAD4);

	// マウスホイールの回転量を取得
	mouseWheelRot_ = GetMouseWheelRotVol();

	lastInputInfo_ = currentInputInfo_;

	// 生データ取得
	std::array<char, 256> keyState;
	GetHitKeyStateAll(keyState.data());
	int mouseState = GetMouseInput();
	int padState = GetJoypadInputState(DX_INPUT_PAD1); // 1コントローラーだけ

	// 生データを入力フラグに反映
	for (auto& inputInfo : inputTable_)
	{
		const auto& eventName = inputInfo.first;

		for (auto& inputState : inputInfo.second)
		{
			switch (inputState.type)
			{
			case PeripheralType::KEYBOAD:
				currentInputInfo_[eventName] = keyState[inputState.id];
				break;
			case PeripheralType::MOUSE:
				currentInputInfo_[eventName] = mouseState & inputState.id;
				break;
			case PeripheralType::PAD:
				currentInputInfo_[eventName] = padState & inputState.id;
				break;
			}
			if (currentInputInfo_[eventName])
			{
				break;
			}
		}
	}

}
bool InputManager::IsPressed(const std::string& name) const
{
	if (!currentInputInfo_.contains(name))
	{
		return false;
	}

	return currentInputInfo_.at(name);
}

bool InputManager::IsTriggerd(const std::string& name) const
{
	if (!currentInputInfo_.contains(name))
	{
		return false;
	}

	return currentInputInfo_.at(name) && !lastInputInfo_.at(name);
}

void InputManager::Destroy(void)
{

	// キー情報のクリア
	keyInfos_.clear();
	mouseInfos_.clear();

	// インスタンスのメモリ解放
	delete instance_;

}

void InputManager::Add(int key)
{
	InputManager::Info info = InputManager::Info();
	info.key = key;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	keyInfos_.emplace(key, info);
}

void InputManager::Clear(void)
{
	keyInfos_.clear();
}

bool InputManager::IsNew(int key) const
{
	return Find(key).keyNew;
}

bool InputManager::IsTrgDown(int key) const
{
	return Find(key).keyTrgDown;
}

bool InputManager::IsTrgUp(int key) const
{
	return Find(key).keyTrgUp;
}

Vector2 InputManager::GetMousePos(void) const
{
	return mousePos_;
}

int InputManager::GetMouse(void) const
{
	return mouseInput_;
}

bool InputManager::IsClickMouseLeft(void) const
{
	return mouseInput_ == MOUSE_INPUT_LEFT;
}

bool InputManager::IsClickMouseRight(void) const
{
	return mouseInput_ == MOUSE_INPUT_RIGHT;
}

bool InputManager::IsTrgMouseLeft(void) const
{
	return FindMouse(MOUSE_INPUT_LEFT).keyTrgDown;
}

bool InputManager::IsTrgMouseRight(void) const
{
	return FindMouse(MOUSE_INPUT_RIGHT).keyTrgDown;
}

InputManager::InputManager(void)
	:
	keyInfos_(),
	mouseInfos_(),
	infoEmpty_(),
	mouseInfoEmpty_(),
	mousePos_(),
	mouseInput_(-1),
	padInfos_(),
	joyDInState_(),
	joyXInState_()
{
}

const InputManager::Info& InputManager::Find(int key) const
{

	auto it = keyInfos_.find(key);
	if (it != keyInfos_.end())
	{
		return it->second;
	}

	return infoEmpty_;

}

const InputManager::MouseInfo& InputManager::FindMouse(int key) const
{
	auto it = mouseInfos_.find(key);
	if (it != mouseInfos_.end())
	{
		return it->second;
	}

	return mouseInfoEmpty_;
}

InputManager::JOYPAD_TYPE InputManager::GetJPadType(JOYPAD_NO no) const
{
	return static_cast<InputManager::JOYPAD_TYPE>(GetJoypadType(static_cast<int>(no)));
}

DINPUT_JOYSTATE InputManager::GetJPadDInputState(JOYPAD_NO no)
{
	// コントローラ情報
	GetJoypadDirectInputState(static_cast<int>(no), &joyDInState_);
	return joyDInState_;
}

XINPUT_STATE InputManager::GetJPadXInputState(JOYPAD_NO no)
{
	// コントローラ情報
	GetJoypadXInputState(static_cast<int>(no), &joyXInState_);
	return joyXInState_;
}

void InputManager::SetJPadInState(JOYPAD_NO jpNo)
{

	int no = static_cast<int>(jpNo);
	auto stateNew = GetJPadInputState(jpNo);
	auto& stateNow = padInfos_[no];

	int max = static_cast<int>(JOYPAD_BTN::MAX);
	for (int i = 0; i < max; i++)
	{

		stateNow.ButtonsOld[i] = stateNow.ButtonsNew[i];
		stateNow.ButtonsNew[i] = stateNew.ButtonsNew[i];

		stateNow.IsOld[i] = stateNow.IsNew[i];
		//stateNow.IsNew[i] = stateNow.ButtonsNew[i] == 128 || stateNow.ButtonsNew[i] == 255;
		stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 0;

		stateNow.IsTrgDown[i] = stateNow.IsNew[i] && !stateNow.IsOld[i];
		stateNow.IsTrgUp[i] = !stateNow.IsNew[i] && stateNow.IsOld[i];

	}

	stateNow.AKeyLX = stateNew.AKeyLX;
	stateNow.AKeyLY = stateNew.AKeyLY;
	stateNow.AKeyRX = stateNew.AKeyRX;
	stateNow.AKeyRY = stateNew.AKeyRY;

}

int InputManager::GetMouseWheelRot(void) const
{
	return mouseWheelRot_;
}

// アナログキーの入力値から方向を取得
VECTOR InputManager::GetDirectionXZAKey(int aKeyX, int aKeyY)
{
	VECTOR ret = { 0.0f, 0.0f, 0.0f };

	// スティックの個々の入力値は、
	// -1000.0f ～ 1000.0f の範囲で返ってくるが、
	// X:1000.0f、Y:1000.0fになることは無い(1000と500くらいが最大)

	// スティックの入力値を -1.0 ～ 1.0 に正規化
	float dirX = static_cast<float>(aKeyX) / AKEY_VAL_MAX;
	float dirZ = static_cast<float>(aKeyY) / AKEY_VAL_MAX;

	// ピタゴラスの定理でニュートラル状態からの長さベクトルにする
	// ( 円形のデッドゾーンになる )

	// 平方根により、おおよその最大値が1.0となる
	float len = sqrtf(dirX * dirX + dirZ * dirZ);
	if (len < THRESHOLD)
	{
		// (0.0f, 0.0f, 0.0f)
		return ret;
	}

	// デッドゾーン境界からに再スケーリング(可変デッドゾーン)
	// ( しきい値 0.35 の場合は、 0.0 ～ 0.65 / 0.65 になる )
	float scale = (len - THRESHOLD) / (1.0f - THRESHOLD);
	dirX = (dirX / len) * scale;
	dirZ = (dirZ / len) * scale;

	// Zは前に倒すとマイナス値が返ってくるので反転
	ret = VNorm({ dirX, 0.0f, -dirZ });

	return ret;
}

