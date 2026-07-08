#include "../../../Application.h"
#include "../../../Scene/SceneManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Object/Common/AnimationController.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderModel.h"
#include "../../Collider/ColliderCapsule.h"
#include "CharactorBase.h"

CharactorBase::CharactorBase(void)
	:
	ActorBase(),
	faceDir_(AsoUtility::DIR_F),
	moveDir_(AsoUtility::VECTOR_ZERO),
	moveSpeed_(),
	movePow_(AsoUtility::VECTOR_ZERO),
	isJump_(false),
	animCtrl_()
{
}

CharactorBase::~CharactorBase(void)
{
}

void CharactorBase::Update(void)
{
	// 移動前座標を更新
	prevPos_ = transform_.pos;

	// 各キャラクターごとの更新処理
	UpdateProcess();

	// 移動方向に応じた遅延回転
	DelayRotate();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定前準備
	CollisionReserve();

	// 衝突判定
	Collision();

	// モデル制御更新
	transform_.Update();

	// アニメーション再生
	animCtrl_->Update();

	// 各キャラクターごとの更新後処理
	UpdateProcessPost();
}

void CharactorBase::Draw(void)
{
	// 基底クラス描画処理
	ActorBase::Draw();
	//DrawFormatString(0, 100, 0xffffff, "isjump:%d", isJump_);
	// 丸影の描画
	//DrawShadow();
}

void CharactorBase::Release()
{
	// アニメコントローラ解放
	if (animCtrl_ != nullptr)
	{
		animCtrl_->Release();
	}

	ActorBase::Release();
}

void CharactorBase::InitLoad(void)
{
	// 丸影画像
	imgShadow_ = resMng_.Load(ResourceManager::SRC::PLAYER_SHADOW).handleId_;
}

void CharactorBase::DelayRotate(void)
{
	// 上方向
	VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

	// 移動方向から回転に変換する
	Quaternion goalRot = Quaternion::LookRotation(faceDir_, upDir);

	// 直接回転
	transform_.quaRot = goalRot;
}

void CharactorBase::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = VNorm(VSub(MOON_CENTER_POS, transform_.pos));

	// 重力の強さ
	float gravityPow = Application::GetInstance().GetGravityPow() * scnMng_.GetDeltaTime();

	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// ---- 追加: 球状重力に対応した終端速度クランプ ----
   // 重力方向の速度成分（正値 = 落下方向）
	float fallSpeed = VDot(jumpPow_, dirGravity);
	if (fallSpeed > MAX_FALL_SPEED)
	{
		// 落下方向成分だけをクランプ、横方向は据え置く
		VECTOR lateralVec = VSub(jumpPow_, VScale(dirGravity, fallSpeed));
		jumpPow_ = VAdd(lateralVec, VScale(dirGravity, MAX_FALL_SPEED));
	}
}

void CharactorBase::Collision(void)
{
	// 移動処理
	transform_.pos = VAdd(transform_.pos, movePow_);

	// 衝突 (カプセル)
	CollisionCapsule();

	// ジャンプ量を加算
	transform_.pos = VAdd(transform_.pos, jumpPow_);

	// 衝突（重力）
	CollisionGravity();
}

void CharactorBase::CollisionGravity(void)
{
	// 線分コライダ
	int lineType = static_cast<int>(COLLIDER_TYPE::GROUND_LINE);

	// 線分コライダが無ければ処理を抜ける
	if (ownColliders_.count(lineType) == 0) return;

	// 線分コライダ情報
	ColliderLine* colliderLine_ =
		dynamic_cast<ColliderLine*>(ownColliders_.at(lineType));

	if (colliderLine_ == nullptr) return;

	// 登録されている衝突物を全てチェック
	for (const auto& hitCol : hitColliders_)
	{
		// ステージ以外は処理を飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;

		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);

		if (colliderModel == nullptr) continue;

		// 上方向
		VECTOR upDir = VNorm(VSub(transform_.pos, MOON_CENTER_POS));

		// 上昇中は衝突判定を発生させない
		// Y軸ではなく、UP方向への速度成分（内積）で上昇中か判断
		float upSpeed = VDot(upDir, jumpPow_);
		if (upSpeed > 0.0f) continue;

		// 衝突したポリゴンの上に押し戻す
		bool isHit = colliderLine_->PushBackUp(
			colliderModel, transform_,
			upDir,  // 上方向い
			2.0f,	// 押し戻し距離
			true,	// ブラックリスト使用許可
			false	// ホワイトリスト使用許可
		);

		// 当たっていたらジャンプフラグを折る
		if (isHit) isJump_ = false;

		if (!isJump_)
		{
			// ジャンプリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;

			// ジャンプの入力受付時間をリセット
			stepJump_ = 0.0f;
		} 
	}
}

void CharactorBase::CollisionCapsule(void)
{
	// カプセルコライダ
	int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);

	// カプセルコライダがなければ処理を抜ける
	if (ownColliders_.count(capsuleType) == 0) return;

	// カプセルコライダ情報
	ColliderCapsule* colliderCapsule =
		dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType));
	if (colliderCapsule == nullptr) return;

	// 登録されている衝突物をすべてチェック
	for (const auto& hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);

		if (colliderModel == nullptr) continue;

		// 指定された回数と距離で三角形の法線方向に押し戻す
		colliderCapsule->PushBackAlongNormal(
			colliderModel, transform_,
			CNT_TRY_COLLISION, COLLISION_BACK_DIS,
			true,	// ブラックリスト使用許可
			false	// ホワイトリスト使用許可
		);
	}
}


