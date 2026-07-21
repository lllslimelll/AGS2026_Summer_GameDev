#include "../../Utility/Math.h"
#include "../../Manager/ResourceManager.h"
#include "../../Game/Scene/SceneManager.h"
#include "../../Utility/AsoUtility.h"
#include "../../Component/StaticMeshComponent.h"
#include "SkyDome.h"

SkyDome::SkyDome(const ActorBase* followTarget)
	: ActorBase()
	, followTarget_(followTarget)
{
}

SkyDome::~SkyDome() {}

void SkyDome::Init(void)
{
	// モデルロード
	int modelId = resMng_.Load(ResourceManager::SRC::SPACE_DOME).handleId_;
	mesh_ = AddComponent<StaticMeshComponent>(modelId);

	// トランスフォーム
	SetScl(Vector3(0.3f, 0.3f, 0.3f));
	SetRot(Quaternion::AngleAxis(Math::ToRadian(200.0f), Vector3::FORWARD));
	if (followTarget_) SetPos(followTarget_->GetPos());

	// Component を Init（CollisionManager への登録等）
	ActorBase::Init();

	// Init後処理（モデルIDが確定してから）
	MV1SetUseZBuffer(modelId, true);
	MV1SetWriteZBuffer(modelId, false);

	SceneManager::SCENE_ID sceneId = scnMng_.GetSceneID();
	ChangeState(sceneId == SceneManager::SCENE_ID::GAME ? STATE::FOLLOW : STATE::STAY);
}

void SkyDome::Update(void)
{
	switch (state_)
	{
	case STATE::STAY:   UpdateStay();   break;
	case STATE::FOLLOW: UpdateFollow(); break;
	default: break;
	}
	ActorBase::Update();
}

void SkyDome::Draw(void)
{
	SetUseLighting(FALSE);
	ActorBase::Draw();
	SetUseLighting(TRUE);
}

void SkyDome::ChangeState(STATE state)
{
	state_ = state;
	if (state_ == STATE::FOLLOW && followTarget_)
		SetPos(followTarget_->GetPos());
}

void SkyDome::UpdateStay(void)
{
	Quaternion rot = Quaternion::AngleAxis(Math::ToRadian(0.07f), Vector3::UP);
	SetRot(GetRot() * rot);
}

void SkyDome::UpdateFollow(void)
{
	Quaternion rot = Quaternion::AngleAxis(Math::ToRadian(0.03f), Vector3::UP);
	SetRot(GetRot() * rot);
	if (followTarget_) SetPos(followTarget_->GetPos());
}