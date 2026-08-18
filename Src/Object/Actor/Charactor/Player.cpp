#include "../../../Object/Common/AnimationController.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Scene/SceneManager.h"
#include "../../../Camera/Camera.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SoundManager.h"
#include "../Item/ItemManager.h"
#include "../Stage/StageManager.h"
#include "../Stage/Rocket.h"
#include "../item/Item.h"
#include "../../Common/Transform.h"
#include "../../Collider/ColliderModel.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include "Player.h"


Player::Player(ItemManager* itemMng, StageManager& stageMng)
	:
	CharactorBase(),
	stageMng_(stageMng),
	itemMgr_(itemMng),
	hp_(MAX_HP),
	oxygen_(MAX_OXYGEN),
	suffocateTimer_(0.0f),
	isBoost_(false),
	aimedItem_(nullptr),
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

	animCtrl_->Play(static_cast<int>(ANIM_TYPE::DEAD), false);
	SoundManager::GetInstance().StopWalk();
	SoundManager::GetInstance().StopAlert();

	stateUpdate_ = []() {};

	// DeadSceneをオーバーレイで表示
	SceneManager::GetInstance().PushOverlay(SceneManager::SCENE_ID::DEAD);
}
void Player::ChangeStateEnd(void)
{
	
}

void Player::UpdateIdle(void)
{
	// 移動・ジャンプ
	ProcessMove();
	//ProcessJump();

	// 当たり判定調整
	CollisionReserve();

	// 酸素とHP
	UpdateOxygenAndHp();
}
void Player::UpdateDead(void)
{
	// Deadアニメーションが終わったらEND状態へ
	if (animCtrl_->IsEnd())
	{
		ChangeState(STATE::END);
		return;
	}

	// mixamorig:Headフレームにカメラを追従させる
	int headFrame = MV1SearchFrame(transform_.modelId, "mixamorig:Head");
	if (headFrame >= 0)
	{
		VECTOR headPos = MV1GetFramePosition(transform_.modelId, headFrame);
		SceneManager::GetInstance().GetCamera().SetDeadCameraPos(headPos);
	}
}

void Player::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	movePow_ = AsoUtility::VECTOR_ZERO;
	VECTOR kDir = AsoUtility::VECTOR_ZERO;
	VECTOR dDir = AsoUtility::VECTOR_ZERO;

	// 入力取得（省略、そのまま）
	if (ins.IsPressed(InputManager::InputCommand::MOVE_FORWARD)) { kDir = VAdd(kDir, AsoUtility::DIR_F); }
	if (ins.IsPressed(InputManager::InputCommand::MOVE_BACK)) { kDir = VAdd(kDir, AsoUtility::DIR_B); }
	if (ins.IsPressed(InputManager::InputCommand::MOVE_LEFT)) { kDir = VAdd(kDir, AsoUtility::DIR_L); }
	if (ins.IsPressed(InputManager::InputCommand::MOVE_RIGHT)) { kDir = VAdd(kDir, AsoUtility::DIR_R); }
	if (!AsoUtility::EqualsVZero(kDir)) { kDir = VNorm(kDir); }
	dDir = ins.GetInstance().GetLeftStickDirection();

	if (ins.IsTriggered(InputManager::InputCommand::BOOST)) { isBoost_ = true; }
	if (isJump_) SoundManager::GetInstance().StopWalk();

	// ---- ここが変更点 ----
	// 上方向は +Y 固定
	const VECTOR upDir = AsoUtility::AXIS_Y;

	// カメラのforward/rightを取得して XZ 平面へ射影
	Quaternion cameraRot = cameraTransform_->quaRot;
	VECTOR camForward = Quaternion::PosAxis(cameraRot, AsoUtility::DIR_F);
	VECTOR camRight = Quaternion::PosAxis(cameraRot, AsoUtility::DIR_R);

	// Y成分を落とすだけで平面射影が完了する（球面射影より単純）
	VECTOR planeForward = { camForward.x, 0.0f, camForward.z };
	VECTOR planeRight = { camRight.x,   0.0f, camRight.z };

	// ゼロ長対策（真上/真下を向いてる時の防護）
	if (VSize(planeForward) < 0.0001f) planeForward = AsoUtility::DIR_F;
	if (VSize(planeRight) < 0.0001f) planeRight = AsoUtility::DIR_R;
	planeForward = VNorm(planeForward);
	planeRight = VNorm(planeRight);

	if (!AsoUtility::EqualsVZero(kDir) || !AsoUtility::EqualsVZero(dDir))
	{
		SoundManager::GetInstance().PlayWalk();

		// 顔向きはカメラforwardに固定（1人称なので体もカメラ方向を向く）
		faceDir_ = planeForward;

		VECTOR moveVec = AsoUtility::VECTOR_ZERO;
		moveVec = VAdd(moveVec, VScale(planeForward, kDir.z));
		moveVec = VAdd(moveVec, VScale(planeRight, kDir.x));
		moveVec = VAdd(moveVec, VScale(planeForward, dDir.z));
		moveVec = VAdd(moveVec, VScale(planeRight, dDir.x));

		moveDir_ = VNorm(moveVec);

		if (isBoost_) {
			moveSpeed_ = SPEED_DASH;
			animCtrl_->Play(static_cast<int>(ANIM_TYPE::FAST_RUN), true);
			SoundManager::GetInstance().StopWalk();

			// ブースト移動開始の瞬間だけ鳴らす
			if (!wasBoostMoving_)
			{
				SoundManager::GetInstance().PlayBoost();
				wasBoostMoving_ = true;  // 追加
			}
		}
		else {
			moveSpeed_ = SPEED_MOVE;
			animCtrl_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
			wasBoostMoving_ = false;
		}

		movePow_ = VScale(moveDir_, moveSpeed_);
	}
	else
	{
		SoundManager::GetInstance().StopWalk();
		wasBoostMoving_ = false;

		// 入力なしでもカメラ方向に体を合わせる
		faceDir_ = planeForward;

		if (!isJump_) { animCtrl_->Play(static_cast<int>(ANIM_TYPE::IDLE), true); }
		isBoost_ = false;
	}

	if (kDir.z < 0.0f || dDir.z < 0.0f) { isBoost_ = false; }
}

void Player::UpdateProcess(void)
{
	// 状態別更新
	stateUpdate_();
}

void Player::UpdateProcessPost(void)
{
	if (state_ == STATE::DEAD || state_ == STATE::END) return;

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

	// ドロップ処理
	ProcessDrop();

	// 照準半径の補間
	float targetRadius = CanPickUp()|| IsAimingRoket() ? 20.0f : 5.0f;
	crosshairRadius_ += 
		(targetRadius - crosshairRadius_) * 15.0f * scnMng_.GetDeltaTime();

	// 選択中アイテムの追従
	//UpdateFollowItem();

	// 帰還
	if (IsAimingRoket())
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
	// 照準に当たっているアイテムを取得
	// RANGE_PICKUP の長さのレイが届く範囲 = 拾える範囲
	Item* newAimed = itemMgr_->GetAimedItem(
		cameraTransform_->pos,
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

	for (int i = 0; i < Inventory::SLOT_MAX; i++)
	{
		Item* item = inventory_.Get(i);
		if (item != nullptr) { item->SetHeldPos(pos); }
	}
}

void Player::ProcessPickUp(void)
{
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
		inventory_.Add(aimedItem_);
	}
}

void Player::ProcessThrow(void)
{
}

void Player::ProcessDrop(void)
{
	auto& ins = InputManager::GetInstance();
	if (!ins.IsTriggered(InputManager::InputCommand::DROP)) return;

	Item* item = inventory_.GetSelected();
	if (item == nullptr) return;

	item->OnDrop(transform_.pos);
	inventory_.RemoveSelected();
}

void Player::DrawFrameRecursive(int modelId, int frameIdx)
{
	MV1DrawFrame(modelId, frameIdx);

	// 子フレームを再帰的に描画
	int childNum = MV1GetFrameChildNum(modelId, frameIdx);
	for (int i = 0; i < childNum; i++)
	{
		int childIdx = MV1GetFrameChild(modelId, frameIdx, i);
		DrawFrameRecursive(modelId, childIdx);
	}
}

void Player::ChangeSelectedSlot()
{
	auto& ins = InputManager::GetInstance();

	// 左右のスロット
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_LEFT)) { inventory_.SelectPrev(); }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_RIGHT)) { inventory_.SelectNext(); }
	// 12345スロット
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_1)) { inventory_.Select(0); }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_2)) { inventory_.Select(1); }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_3)) { inventory_.Select(2); }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_4)) { inventory_.Select(3); }
	if (ins.IsTriggered(InputManager::InputCommand::SLOT_5)) { inventory_.Select(4); }
	// マウスホイール
	int wheel = ins.GetMouseWheelRot();
	if (wheel > 0) { inventory_.SelectPrev(); }
	else if (wheel < 0) { inventory_.SelectNext(); }
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

	// 満杯なら拾えない
	return !inventory_.IsFull();
}

void Player::Draw(void)
{
	//ActorBase::Draw();

	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

#ifdef DEBUG
	DrawFormatString(0,0,0xffffff,"Player pos : (%.1f, %.1f, %.1f)\n",
		transform_.pos.x, transform_.pos.y, transform_.pos.z);
#endif // DEBUG 


	// 照準
	int cx = screenW / 2;
	int cy = screenH / 2;
	int radius = static_cast<int>(crosshairRadius_);
	bool isFilled = (crosshairRadius_ <= 7.0f);

	DrawCircle(cx, cy, radius, GetColor(255, 255, 255), isFilled ? TRUE : FALSE);
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

const Inventory& Player::GetInventory(void) const
{
	return inventory_;
}

Player::GUIDE_INFO Player::GetGuideInfo(void) const
{
	GUIDE_INFO info;
	info.canPickUp = CanPickUp();
	info.isAimingRocket = IsAimingRoket();
	info.hasSelectedItem = inventory_.GetSelected() != nullptr;
	info.isIdle = state_ == STATE::IDLE;
	info.isPad = GetJoypadNum() != 0;
	info.hasAnyItem = inventory_.HasAnyItem();
	info.totalDelivered = stageMng_.GetRocket().GetTotalDelivered();
	info.quota = Rocket::QUOTA;
	return info;
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
	Item* item = inventory_.GetSelected();
	if (item == nullptr) return;

	auto& ins = InputManager::GetInstance();
	if (!ins.IsPressed(InputManager::InputCommand::PICK_UP)) return;

	// 納品実行
	stageMng_.GetRocket().AddDelivery(item->GetValue());
	item->OnDelivered();
	inventory_.RemoveSelected();
}

bool Player::IsAimingRoket(void) const
{
	const VECTOR camPos = cameraTransform_->pos;
	const VECTOR rayEnd = VAdd(camPos,
		VScale(cameraForward_, Item::RANGE_PICKUP_ROCKET));

	const ColliderModel* colModel = dynamic_cast<const ColliderModel*>(
		stageMng_.GetRocket().GetOwnCollider(
			static_cast<int>(Rocket::COLLIDER_TYPE::MODEL)));
	
	if (colModel == nullptr) { return false; }

	return colModel->IsHitRay(camPos, rayEnd);
}

void Player::UpdateOxygenAndHp(void)
{
	if (state_ == STATE::DEAD || state_ == STATE::END) return;

	const float dt = SceneManager::GetInstance().GetDeltaTime();

	// 酸素を減らす
	// 酸素消費倍率
	float consumeRate = isBoost_ ? OXYGEN_DASH_RATE : 1.0f;
	oxygen_ -= dt * consumeRate;
	if (oxygen_ < 0.0f) oxygen_ = 0.0f;

	// 酸素が残っているなら窒息タイマーをリセット
	if (oxygen_ > 0.0f)
	{
		suffocateTimer_ = 0.0f;

		isAlertPlaying_ = false;
		return;
	}

	// 酸素切れ：初回だけAlertを鳴らす
	if (!isAlertPlaying_)
	{
		SoundManager::GetInstance().PlayAlert();
		isAlertPlaying_ = true;
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

void Player::OnDamagedByEnemy(int damage)
{
	hp_ -= damage;

	SoundManager::GetInstance().PlayDamaged2();

	if (hp_ <= 0)
	{
		hp_ = 0;
		ChangeState(STATE::DEAD);
	}
}

void Player::InitLoad(void)
{
	// モデル読み込み
	transform_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::PLAYER).handleId_);
}

void Player::InitTransform(void)
{
	// 大きさ
	transform_.scl = AsoUtility::VECTOR_ONE;

	// Y軸を180度
	transform_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(180), AsoUtility::AXIS_Y);

	// 座標
	transform_.pos = { -1100, 7.2f, -7622.0f };

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

	animCtrl_->Add(static_cast<int>(ANIM_TYPE::DEAD), 20.0f, resMng_.Load(ResourceManager::SRC::DEAD).path_);
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

