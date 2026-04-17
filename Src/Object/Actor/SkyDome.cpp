#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../Common/Transform.h"
#include "../../Utility/AsoUtility.h"
#include "SkyDome.h"

SkyDome::SkyDome(const Transform& followTransform)
	:
	followTransform_(followTransform),
	state_(STATE::FOLLOW),
	ActorBase()
{
}

SkyDome::~SkyDome()
{
}

void SkyDome::Update(void)
{
	switch (state_)
	{
	case SkyDome::STATE::NONE:
		UpdateNone();
		break;
	case SkyDome::STATE::STAY:
		UpdateStay();
		break;
	case SkyDome::STATE::FOLLOW:
		UpdateFollow();
		break;
	}


	// 空をY軸に毎フレーム0.05°ずつ回転を追加
	/*transform_.quaRotLocal = Quaternion::Mult(
		transform_.quaRotLocal,
		Quaternion::AngleAxis(AsoUtility::Deg2RadF(0.05f), AsoUtility::AXIS_Y));*/

	transform_.Update();
}

void SkyDome::Draw(void)
{
	SetUseLighting(FALSE);
	MV1DrawModel(transform_.modelId);
	SetUseLighting(TRUE);
}

void SkyDome::InitLoad(void)
{
	// モデル読み込み
	transform_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::SKY_DOME).handleId_);
}

void SkyDome::InitTransform(void)
{
	transform_.scl = { 100.0f,100.0f,100.0f };

	transform_.quaRot = Quaternion::Identity();
	// Y軸を180度
	transform_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);

	// 座標
	transform_.pos = AsoUtility::VECTOR_ZERO;
}

void SkyDome::InitCollider(void)
{
}

void SkyDome::InitAnimation(void)
{
}

void SkyDome::InitPost(void)
{
	transform_.Update();

	// Zバッファ向こう（突き抜け対策）
	MV1SetUseZBuffer(transform_.modelId, true);
	MV1SetWriteZBuffer(transform_.modelId, false);

	// 初期状態
	SceneManager::SCENE_ID sceneId = scnMng_.GetSceneID();
	if (sceneId == SceneManager::SCENE_ID::GAME)
	{
		ChangeState(STATE::FOLLOW);
	}
	else
	{
		ChangeState(STATE::STAY);
	}
}

void SkyDome::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	// 状態ごとの初期化処理
	switch (state_)
	{
	case SkyDome::STATE::NONE:
		ChangeStateNone();
		break;
	case SkyDome::STATE::STAY:
		ChangeStateStay();
		break;
	case SkyDome::STATE::FOLLOW:
		ChangeStateFollow();
		break;
	}
}

void SkyDome::ChangeStateNone(void)
{
}

void SkyDome::ChangeStateStay(void)
{
}

void SkyDome::ChangeStateFollow(void)
{
	// 追従開始
	transform_.pos = followTransform_.pos;
	transform_.Update();
}

void SkyDome::UpdateNone(void)
{
}

void SkyDome::UpdateStay(void)
{
	// Y軸回転
	Quaternion rot = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(0.1f), AsoUtility::AXIS_Y);
	transform_.quaRot = transform_.quaRot.Mult(rot);
	transform_.Update();
}

void SkyDome::UpdateFollow(void)
{
	// Y軸回転
	Quaternion rot = Quaternion::AngleAxis(
		AsoUtility::Deg2RadF(0.1f), AsoUtility::AXIS_Y);
	transform_.quaRot = transform_.quaRot.Mult(rot);

	// 追従
	transform_.pos = followTransform_.pos;

	transform_.Update();
}
