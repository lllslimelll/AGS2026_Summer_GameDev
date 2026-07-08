#include "../../../Object/Common/AnimationController.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Scene/SceneManager.h"
#include "../../../Camera/Camera.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SoundManager.h"
#include "../Item/ItemManager.h"
#include "../Stage.h"
#include "../item/Item.h"
#include "../../Common/Transform.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include "Player.h"


Player::Player(ItemManager* itemMng, Stage* stage)
	:
	CharactorBase(),
	stage_(stage),
	itemMgr_(itemMng),
	hp_(MAX_HP),
	oxygen_(MAX_OXYGEN),
	suffocateTimer_(0.0f),
	isBoost_(false),
	aimedItem_(nullptr),
	inventory_{},
	selectedSlot_(0),
	crosshairRadius_(5.0f)
{
}

Player::~Player(void)
{
}

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

	SoundManager::GetInstance().StopWalk();
}
void Player::ChangeStateEnd(void)
{
	// 死亡シーンのオーバーレイを追加
	SceneManager::GetInstance().PushOverlay(SceneManager::SCENE_ID::DEAD);
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
}

void Player::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	// 移動量
	movePow_ = AsoUtility::VECTOR_ZERO;
	// 移動方向
	VECTOR kDir = AsoUtility::VECTOR_ZERO;
	VECTOR dDir = AsoUtility::VECTOR_ZERO;

	// 移動操作（キーボード）
	if (ins.IsPressed(InputManager::InputCommand::MOVE_FORWARD)) { kDir = VAdd(kDir, AsoUtility::DIR_F); }
	if (ins.IsPressed(InputManager::InputCommand::MOVE_BACK))	 { kDir = VAdd(kDir, AsoUtility::DIR_B); }
	if (ins.IsPressed(InputManager::InputCommand::MOVE_LEFT))	 { kDir = VAdd(kDir, AsoUtility::DIR_L); }
	if (ins.IsPressed(InputManager::InputCommand::MOVE_RIGHT))	 { kDir = VAdd(kDir, AsoUtility::DIR_R); }
	// 斜め入力時に正規化（長さを1に統一）
	if (!AsoUtility::EqualsVZero(kDir)) { kDir = VNorm(kDir); }

	// 移動操作（ゲームパッド）
	dDir = ins.GetInstance().GetLeftStickDirection();
	
	// ブースト
	if (ins.IsTriggered(InputManager::InputCommand::BOOST)) { isBoost_ = true; }

	if(isJump_) SoundManager::GetInstance().StopWalk();

	// 移動入力がある場合
	if (!AsoUtility::EqualsVZero(kDir) || !AsoUtility::EqualsVZero(dDir))
	{
		SoundManager::GetInstance().PlayWalk();

		// 上方向を取得（地面の法線方向）
		VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

		// カメラの回転を取得
		Quaternion cameraRot = cameraTransform_->quaRot;

		// カメラの「前」と「右」をベクトルとして取り出す
		VECTOR camForward = Quaternion::PosAxis(cameraRot, AsoUtility::DIR_F);
		VECTOR camRight = Quaternion::PosAxis(cameraRot, AsoUtility::DIR_R);

		// 星の表面に沿わせる
		float dotF = VDot(camForward, upDir);
		VECTOR surfaceForward = VNorm(VSub(camForward, VScale(upDir, dotF)));

		float dotR = VDot(camRight, upDir);
		VECTOR surfaceRight = VNorm(VSub(camRight, VScale(upDir, dotR)));

		// キャラの向く方向をカメラの前方に固定
		faceDir_ = surfaceForward;

		// dirに合わせて移動ベクトルを合成
		VECTOR moveVec = AsoUtility::VECTOR_ZERO;
		moveVec = VAdd(moveVec, VScale(surfaceForward, kDir.z)); // 左右の移動を加算（キーボード）
		moveVec = VAdd(moveVec, VScale(surfaceRight, kDir.x));	 // 前後の移動を加算（キーボード）
		moveVec = VAdd(moveVec, VScale(surfaceForward, dDir.z)); // 左右の移動を加算（ゲームパッド）
		moveVec = VAdd(moveVec, VScale(surfaceRight, dDir.x));	 // 前後の移動を加算（ゲームパッド）

		// 移動ベクトルの正規化
		moveDir_ = VNorm(moveVec);

		if (isBoost_)
		{
			// ダッシュ速度
			moveSpeed_ = SPEED_DASH;

			// 速く走るアニメーション再生
			animCtrl_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN), true);

			//SoundManager::GetInstance().PlayBoost();
		}
		else
		{
			// 歩行速度
			moveSpeed_ = SPEED_MOVE;

			// 走るアニメーション再生
			animCtrl_->Play(static_cast<int>(ANIM_TYPE::RUN), true);

			//SoundManager::GetInstance().StopBoost();
		}

		// 移動速度を反映
		movePow_ = VScale(moveDir_, moveSpeed_);
	}
	else // 移動入力がない場合
	{
		SoundManager::GetInstance().StopWalk();

		// 入力がない時も、常にカメラの方向を向かせ続ける場合
		VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));
		Quaternion cameraRot = cameraTransform_->quaRot;
		VECTOR camForward = Quaternion::PosAxis(cameraRot, AsoUtility::DIR_F);
		float dotF = VDot(camForward, upDir);
		faceDir_ = VNorm(VSub(camForward, VScale(upDir, dotF)));

		// ジャンプ中はアニメーションを変えない
		if (!isJump_) { animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true); }

		// ブーストフラグを折る
		isBoost_ = false;
	}
	
	// 後退中はブーストを無効化
	if(kDir.z < 0.0f || dDir.z < 0.0f) { isBoost_ = false; }
}

void Player::ProcessJump(void)
{
	auto& ins = InputManager::GetInstance();

	// プレイヤーの上方向を計算（地面の法線方向）
	VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

	// ジャンプキーが押されたか
	bool isHitKey = ins.IsTriggered(InputManager::InputCommand::JUMP);
	// ジャンプキーが押されているか
	bool isHitKeyNew = ins.IsPressed(InputManager::InputCommand::JUMP);

	// 初期ジャンプ処理
	if (isHitKey && !isJump_)
	{
		// ジャンプ量の計算
		float jumpSpeed = POW_JUMP_INIT * scnMng_.GetDeltaTime();
		jumpPow_ = VScale(upDir, jumpSpeed);
		stepJump_ = 0.0f;
		isJump_ = true;

		// アニメーション再生
		animCtrl_->Play(
			static_cast<int>(ANIM_TYPE::JUMP), false);
	}

	// 持続ジャンプ処理（長押し）
	if (isHitKeyNew)
	{
		// ジャンプの入力受付時間を減少
		stepJump_ += scnMng_.GetDeltaTime();
		if(stepJump_ < TIME_JUMP_INPUT)
		{
			// ジャンプ量の計算
			float jumpSpeed = POW_JUMP_KEEP * scnMng_.GetDeltaTime();
			jumpPow_ = VAdd(jumpPow_, VScale(upDir, jumpSpeed));
		}
	}
	else
	{
		// ボタンを離したらジャンプ力に加算しない
		stepJump_ = TIME_JUMP_INPUT;
	}
}

void Player::UpdateProcess(void)
{
	// 状態別更新
	stateUpdate_();
}

void Player::UpdateProcessPost(void)
{
	if (state_ == STATE::DEAD) return;

	// アイテム更新
	UpdateItem();

	// 選択スロットの変更
	ChangeSelectedSlot();
}


void Player::UpdateItem(void)
{
	// 照準アイテムの更新
	UpdateAimedItem();

	// 拾う処理
	ProcessPickUp();

	// 納品処理
	ProcessDelivery();

	// 投擲処理
	ProcessThrow();

	// 照準半径の補間
	float targetRadius = CanPickUp()|| IsAimingRoket() ? 20.0f : 5.0f;
	crosshairRadius_ += 
		(targetRadius - crosshairRadius_) * 15.0f * scnMng_.GetDeltaTime();

	// 選択中アイテムの追従
	UpdateFollowItem();

	// 帰還
	if (IsAimingRoket())
	{
		auto& ins = InputManager::GetInstance();
		
		if (ins.IsTriggered(InputManager::InputCommand::RETURN))
		{
			//SceneManager::GetInstance().SetResultScore(stage_->GetTotalDelivered());
			SceneManager::GetInstance().PushOverlay(SceneManager::SCENE_ID::RESULT);
		}
	}
}

void Player::UpdateAimedItem(void)
{
	// 照準に当たっているアイテムを取得
	// RANGE_PICKUP の長さのレイが届く範囲 = 拾える範囲
	Item* newAimed = itemMgr_->GetAimedItem(
		transform_.pos,
		cameraForward_,
		Item::RANGE_PICKUP);

	// 前フレームと変わった場合のみ SetAimed を呼ぶ
	if (newAimed != aimedItem_)
	{
		// 前フレームのアイテムのエイムを解除
		if (aimedItem_ != nullptr)
		{
			aimedItem_->SetAimed(false);
		}

		// 新しいアイテムにエイムをセット
		if (newAimed != nullptr)
		{
			newAimed->SetAimed(true);
		}

		aimedItem_ = newAimed;
	}
}

void Player::UpdateFollowItem(void)
{
	VECTOR pos = MV1GetFramePosition(transform_.modelId, 36);
	pos = VAdd(pos, VScale(transform_.GetForward(), 30.0f));

	for (int i = 0; i < INVENTORY_MAX; i++)
	{
		if (inventory_[i] != nullptr)
		{
			inventory_[i]->SetHeldPos(pos);
		}
	}

}

void Player::ProcessPickUp(void)
{
	//// 拾える状態じゃなければ処理しない
	//if (!CanPickUp()) return;

	//auto& ins = InputManager::GetInstance();

	//// 入力があったら
	//if (ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT) || ins.IsTrgDown(KEY_INPUT_F))
	//{
	//	// 拾う
	//	aimedItem_->OnPickedUp();

	//	// インベントリに追加
	//	AddInventory(aimedItem_);
	//}

	   // 照準にアイテムが当たってない or ドロップ状態でないなら何もしない
	if (aimedItem_ == nullptr ||
		aimedItem_->GetState() != Item::STATE::DROPPED)
	{
		return;
	}

	auto& ins = InputManager::GetInstance();

	if (!ins.IsTriggered(InputManager::InputCommand::PICK_UP)) return;

	if (CanPickUp())
	{
		// 拾う
		aimedItem_->OnPickedUp();
		AddInventory(aimedItem_);
	}
}

void Player::ProcessThrow(void)
{
}

void Player::AddInventory(Item* item)
{
	// 取得したアイテムを空いているスロットに追加
	for (int i = 0; i < INVENTORY_MAX; i++)
	{
		if (inventory_[i] == nullptr)
		{
			inventory_[i] = item;

			break;
		}
	}
}

void Player::ChangeSelectedSlot()
{
	// 旧選択アイテムをデタッチ
	if (inventory_[selectedSlot_] != nullptr)
	{
		inventory_[selectedSlot_]->SetSelected(false);
	}

	auto& ins = InputManager::GetInstance();
	
	// インベントリスロットの変更
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_LEFT))
	{
		selectedSlot_ = (selectedSlot_ - 1 + INVENTORY_MAX) % INVENTORY_MAX;
	}
	else if (ins.IsTriggered(InputManager::InputCommand::SLOT_RIGHT))
	{
		selectedSlot_ = (selectedSlot_ + 1) % INVENTORY_MAX;
	}

	// 12345キー
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_1)) { selectedSlot_ = 0; }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_2)) { selectedSlot_ = 1; }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_3)) { selectedSlot_ = 2; }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_4)) { selectedSlot_ = 3; }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_5)) { selectedSlot_ = 4; }

	// マウスホイール
	int wheel = ins.GetMouseWheelRot();
	if (wheel > 0) { selectedSlot_ = (selectedSlot_ - 1 + INVENTORY_MAX) % INVENTORY_MAX; }
	else if (wheel < 0) { selectedSlot_ = (selectedSlot_ + 1) % INVENTORY_MAX; }

	// 新選択アイテムをアタッチ
	if (inventory_[selectedSlot_] != nullptr)
	{
		inventory_[selectedSlot_]->SetSelected(true);
	}
}

bool Player::CanPickUp(void) const
{
	// 照準に当たってるアイテムなかったら or 
	// アイテムがドロップ状態じゃなければ
	if (aimedItem_ == nullptr ||
		aimedItem_->GetState() != Item::STATE::DROPPED)
	{
		// 拾えない
		return false;
	}

	// インベントリに空きがあるか
	for (int i = 0; i < INVENTORY_MAX; i++)
	{
		// 空きがあれば
		if (inventory_[i] == nullptr)
		{
			// 拾える
			return true;
		}
	}

	// 空きがなければ拾えない
	return false;
}

void Player::Draw(void)
{
	// 基底クラスの描画
	CharactorBase::Draw();

	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	const int size = 130;
	const int imgSize = 110;
	const int padding = 30;
	const int slotSpan = size + padding;
	const int totalWidth = INVENTORY_MAX * slotSpan - padding;
	const int startX = (screenW - totalWidth) / 2 + 10;
	const int marginBottom = 60;
	const int baseY = screenH - size - marginBottom;

	// 選択中スロットの拡大量
	const int selectedExpand = 30;

	for (int i = 0; i < INVENTORY_MAX; i++)
	{
		int x = startX + i * slotSpan;

		bool isSelected = (i == selectedSlot_);

		int expand = isSelected ? selectedExpand : 0;
		int drawX = x - expand / 2;
		int drawY = baseY - expand / 2;
		int drawSize = size + expand;

		// 枠描画
		DrawBox(drawX, drawY, drawX + drawSize, drawY + drawSize,
			GetColor(255, 255, 255), FALSE);

		// アイテム画像
		Item* item = inventory_[i];
		if (item != nullptr)
		{
			int typeIdx = static_cast<int>(item->GetType());
			if (typeIdx >= 0 && typeIdx < 3 &&
				inventoryItemImgs_[typeIdx] != -1)
			{
				int margin = 10;              // 枠からの余白
				int imgDrawSize = drawSize - margin * 2;
				int imgOffset = margin;

				DrawExtendGraph(
					drawX + imgOffset,
					drawY + imgOffset,
					drawX + imgOffset + imgDrawSize,
					drawY + imgOffset + imgDrawSize,
					inventoryItemImgs_[typeIdx], TRUE);
			}

			SetFontSize(30);
			// 値段表示
			int price = item->GetValue();
			int textW = GetDrawFormatStringWidth("$%d", price);  // 実際の文字列幅を取得
			DrawFormatString(
				drawX + (drawSize - textW) / 2,  // 中央揃え
				drawY + drawSize + 6,
				GetColor(255, 255, 255),
				"$%d", price);
		}
	}

	// 照準
	int cx = screenW / 2;
	int cy = screenH / 2;
	int radius = static_cast<int>(crosshairRadius_);
	bool isFilled = (crosshairRadius_ <= 7.0f);

	DrawCircle(cx, cy, radius, GetColor(255, 255, 255), isFilled ? TRUE : FALSE);

	DrawControlHelp();


	/*DrawFormatString(0, 0, GetColor(255, 255, 255),
		"(pPosX:%.1f pPosY:%.1f pPosZ:%.1f)",
		transform_.pos.x, transform_.pos.y, transform_.pos.z);*/
}

int Player::GetHp(void) const
{
	return hp_;
}

float Player::GetOxygen(void) const
{
	return oxygen_;
}

void Player::SetCameraTransform(const Transform* cameraTransform)
{
	cameraTransform_ = cameraTransform;
}

void Player::SetForward(const VECTOR forward)
{
	cameraForward_ = forward;
}

// 衝突判定用の調整
void Player::CollisionReserve(void)
{
	//// アニメーションごとの線分調整
	//if (animCtrl_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP))
	//{
	//	// ジャンプ中は線分を伸ばす
	//	if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)) != 0)
	//	{
	//		ColliderLine* colLine = dynamic_cast<ColliderLine*>(
	//			ownColliders_.at(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)));
	//		colLine->SetLocalPosStart(COL_LINE_JUMP_START_LOCAL_POS);
	//		colLine->SetLocalPosEnd(COL_LINE_JUMP_END_LOCAL_POS);
	//	}
	//}
	//else
	{
		// 通常時の線分に戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::GROUND_LINE)));
			colLine->SetLocalPosStart(COL_LINE_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_END_LOCAL_POS);
		}
	}
	// inplaceアニメーションに変えたらここも削除！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

	// アニメーションごとのカプセル調整
	if (animCtrl_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP))
	{
		// ジャンプ中はカプセルを伸ばす
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::CAPSULE)) != 0)
		{
			ColliderCapsule* colCapsule = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

			colCapsule->SetLocalPosTop(COL_CAPSULE_TOP_JUMP_LOCAL_POS);
			colCapsule->SetLocalPosDown(COL_CAPSULE_DOWN_JUMP_LOCAL_POS);
		}
	}
	else
	{
		// 通常時のカプセルに戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::CAPSULE)) != 0)
		{
			ColliderCapsule* colCapsule = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

			colCapsule->SetLocalPosTop(COL_CAPSULE_TOP_LOCAL_POS);
			colCapsule->SetLocalPosDown(COL_CAPSULE_DOWN_LOCAL_POS);
		}
	}
}

void Player::ProcessDelivery(void)
{
	// 拾える対象がいるならピックアップを優先
	if (aimedItem_ != nullptr) return;

	// ロケットに照準が当たっているか
	if (!IsAimingRoket()) return;

	// 選択中スロットが空なら納品できない
	Item* item = inventory_[selectedSlot_];
	if (item == nullptr) return;

	// ボタン入力（ピックアップと同じキー）
	auto& ins = InputManager::GetInstance();

	if (!ins.IsPressed(InputManager::InputCommand::PICK_UP)) return;

	// 納品実行
	stage_->AddDelivery(item->GetValue());
	item->OnDelivered();
	inventory_[selectedSlot_] = nullptr;
}

bool Player::IsAimingRoket(void) const
{
	const VECTOR camPos = cameraTransform_->pos;
	const VECTOR rayEnd = VAdd(camPos,
		VScale(cameraForward_, Item::RANGE_PICKUP));

	VECTOR pos = VAdd(stage_->GetRoketPos(), { 120,-95, 40 });
	return AsoUtility::IsHitSphereCapsule(
		pos, 100.0f,
		camPos, rayEnd, 0.0f);
}

void Player::UpdateOxygenAndHp(void)
{
	if (state_ == STATE::DEAD) return;

	const float dt = SceneManager::GetInstance().GetDeltaTime();

	// 酸素を減らす
	// 酸素消費倍率（ブースト中は2倍）
	float consumeRate = isBoost_ ? OXYGEN_DASH_RATE : 1.0f;
	oxygen_ -= dt * consumeRate;
	if (oxygen_ < 0.0f) oxygen_ = 0.0f;

	// 酸素が残っているなら窒息タイマーをリセット
	if (oxygen_ > 0.0f)
	{
		suffocateTimer_ = 0.0f;
		return;
	}

	// 酸素切れ：タイマー加算
	suffocateTimer_ += dt;
	if (suffocateTimer_ >= SUFFOCATE_INTERVAL)
	{
		suffocateTimer_ -= SUFFOCATE_INTERVAL;  // 余剰時間は次周期に持ち越し
		OnDamaged(SUFFOCATE_DAMAGE);
	}
}

void Player::OnDamaged(int damage)
{
	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		ChangeState(STATE::DEAD);
	}
}

void Player::DrawControlHelp(void)
{
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	bool isPad = (GetJoypadNum() != 0);

	// 表示中のヒントを上から順に積み上げる
	const char* visibleLabels[3];
	int visibleCount = 0;

	// 拾う：照準先がドロップ状態のアイテムで、かつ拾える状態の時のみ
	if (aimedItem_ != nullptr &&
		aimedItem_->GetState() == Item::STATE::DROPPED &&
		CanPickUp())
	{
		visibleLabels[visibleCount++] = isPad ? "拾う : [X]" : "拾う : [F]";
	}

	// ロケットに照準が当たっている時のみ
	if (IsAimingRoket())
	{
		// 納品：選択中スロットにアイテムがある時のみ
		if (inventory_[selectedSlot_] != nullptr)
		{
			visibleLabels[visibleCount++] = isPad ? "納品 : [X]" : "納品 : [F]";
		}

		// 帰還：常時（ロケット照準中であれば）
		visibleLabels[visibleCount++] = isPad ? "帰還 : [Y]" : "帰還 : [E]";
	}

	if (visibleCount == 0) return;

	int prevSize = GetFontSize();
	constexpr int FONT_SIZE = 50;
	constexpr int LINE_SPAN = 60;
	constexpr int MARGIN_TOP = 170;   // Stage側のトータル値段表示と重ならないよう下にオフセット
	constexpr int MARGIN_RIGHT = 60;

	SetFontSize(FONT_SIZE);

	for (int i = 0; i < visibleCount; i++)
	{
		const char* buf = visibleLabels[i];
		int textW = GetDrawStringWidth(buf, (int)strlen(buf));
		int x = screenW - textW - MARGIN_RIGHT;
		int y = MARGIN_TOP + i * LINE_SPAN;

		DrawString(x, y, buf, GetColor(255, 255, 255));
	}

	SetFontSize(prevSize);
}

void Player::InitLoad(void)
{
	// 基底クラスのリソースロード
	CharactorBase::InitLoad();

	// モデル読み込み
	transform_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::PLAYER).handleId_);

	// インベントリ内アイテムの画像読み込み
	inventoryItemImgs_[0] = LoadGraph("Data/Image/item1.png");
	inventoryItemImgs_[1] = LoadGraph("Data/Image/item2.png");
	inventoryItemImgs_[2] = LoadGraph("Data/Image/item3.png");
	
}

void Player::InitTransform(void)
{
	// 大きさ
	transform_.scl = AsoUtility::VECTOR_ONE;
	
	transform_.quaRot = Quaternion::Identity();
	// Y軸を180度
	transform_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);

	// 座標
	transform_.pos = { 1386, -910, -94.8f };

	transform_.Update();

}

void Player::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::GROUND_LINE), colLine);

	// 主に壁や木などの衝突で使用するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS, COL_CAPSULE_RADIUS);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
	// アニメーション
	animCtrl_ = new AnimationController(transform_.modelId);
	animCtrl_->Add(static_cast<int>(ANIM_TYPE::IDLE), 20.0f, resMng_.Load(ResourceManager::SRC::IDLE).path_);

	animCtrl_->Add(static_cast<int>(ANIM_TYPE::RUN), 20.0f, resMng_.Load(ResourceManager::SRC::RUN).path_);

	animCtrl_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN), 20.0f, resMng_.Load(ResourceManager::SRC::FAST_RUN).path_);

	animCtrl_->Add(static_cast<int>(ANIM_TYPE::JUMP), 20.0f, resMng_.Load(ResourceManager::SRC::JUMP_RISING).path_);
}

void Player::InitPost(void)
{
	transform_.Update();

	animCtrl_->Play(0, true);

	// 状態遷移初期処理登録
	stateChanges_.emplace(static_cast<int>(STATE::IDLE),
		std::bind(&Player::ChangeStateIdle, this));

	stateChanges_.emplace(static_cast<int>(STATE::DEAD),
		std::bind(&Player::ChangeStateDead, this));

	stateChanges_.emplace(static_cast<int>(STATE::END),
		std::bind(&Player::ChangeStateEnd, this));

	// 初期状態設定
	ChangeState(STATE::IDLE);
}

