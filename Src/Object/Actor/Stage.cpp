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
	//// Y軸回転
	//Quaternion rot = Quaternion::AngleAxis(
	//	AsoUtility::Deg2RadF(0.1f), AsoUtility::AXIS_Y);
	//transform_.quaRot = transform_.quaRot.Mult(rot);
	transform_.Update();
}

void Stage::Draw(void)
{
	ActorBase::Draw();

	DrawSphere3D(roketPos_, 80, 16, 0xffffff, 0xffffff, false);

	// フォントサイズを一時的に変更
	int prevSize = GetFontSize();
	SetFontSize(48);

	// 文字列の幅を計測して右上揃え
	char buf[64];
	sprintf_s(buf, "$%d / $%d", totalDelivered_, QUOTA);
	int textW = GetDrawStringWidth(buf, (int)strlen(buf));

	constexpr int SCREEN_W = 1920;
	constexpr int MARGIN = 30;
	int x = SCREEN_W - textW - MARGIN;
	int y = MARGIN;

	unsigned int color = IsQuotaCleared() ? 0x00ff00 : 0xffffff;
	DrawFormatString(x, y, color, "%s", buf);

	// フォントサイズを戻す
	SetFontSize(prevSize);
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

	// 座標
	roketPos_ = VAdd(AsoUtility::VECTOR_ZERO, VScale(AsoUtility::DIR_U, 2500.0f));
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

const VECTOR& Stage::GetRoketPos() const
{
	return roketPos_;
}

void Stage::AddDelivery(int value)
{
	totalDelivered_ += value;
}

int Stage::GetTotalDelivered(void)
{
	return totalDelivered_;
}

bool Stage::IsQuotaCleared(void) const
{
	return totalDelivered_ >= QUOTA;
}
