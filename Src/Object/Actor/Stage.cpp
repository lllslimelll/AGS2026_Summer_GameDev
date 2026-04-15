#include "../../Manager/ResourceManager.h"
#include "../Common/Transform.h"
#include "../../Utility/AsoUtility.h"
#include "../Collider/ColliderModel.h"
#include "Stage.h"

//Stage::Stage(void)
//	:
//	ActorBase()
//{
//}

Stage::~Stage(void)
{
}

void Stage::Update(void)
{
	transform_.Update();
}

void Stage::InitLoad(void)
{
	// モデル読み込み
	transform_.SetModel(resMng_.Load(			// 1個 = Load()  複数 = Depulicate()
		ResourceManager::SRC::MAIN_STAGE).handleId_);
}

void Stage::InitTransform(void)
{
	// 大きさ
	transform_.scl = AsoUtility::VECTOR_ONE;
	// 座標
	transform_.pos = { 0.0f, 0.0f, 0.0f };

}

void Stage::InitCollider(void)
{
	// DxLib側の衝突情報セットアップ
	MV1SetupCollInfo(transform_.modelId);

	// モデルのコライダ
	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);

	// 除外フレーム設定
	for (const std::string& name : EXCLUDE_FRAME_NAMES)
	{
		colModel->AddExcludeFrameIds(name);
	}

	// 対象フレーム設定
	for (const std::string& name : TARGET_FRAME_NAMES)
	{ 
		colModel->AddTargetFrameIds(name);
	}

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Stage::InitAnimation(void)
{
}

void Stage::InitPost(void)
{
	transform_.Update();
}
