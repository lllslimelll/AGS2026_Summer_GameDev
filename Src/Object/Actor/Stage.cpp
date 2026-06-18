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

	DrawSphere3D(roketPos_, 120, 16, 0xffffff, 0xffffff, false);

	DrawUI();
}

void Stage::DrawUI(void)
{
	int prevSize = GetFontSize();
	SetFontSize(55);

	// 整数をカンマ区切り文字列に変換するラムダ
	auto withComma = [](int value, char* out)
		{
			char tmp[32];
			sprintf_s(tmp, 32, "%d", value);
			int len = (int)strlen(tmp);
			int outIdx = 0;
			int firstLen = len % 3;
			if (firstLen == 0) firstLen = 3;
			for (int i = 0; i < firstLen; i++) out[outIdx++] = tmp[i];
			for (int i = firstLen; i < len; i += 3)
			{
				out[outIdx++] = ',';
				out[outIdx++] = tmp[i];
				out[outIdx++] = tmp[i + 1];
				out[outIdx++] = tmp[i + 2];
			}
			out[outIdx] = '\0';
		};

	char totalStr[32];
	char quotaStr[32];
	withComma(totalDelivered_, totalStr);
	withComma(QUOTA, quotaStr);

	char buf[64];
	sprintf_s(buf, "$%s / $%s", totalStr, quotaStr);
	int textW = GetDrawStringWidth(buf, (int)strlen(buf));

	constexpr int SCREEN_W = 1920;
	constexpr int MARGIN = 50;
	int x = SCREEN_W - textW - MARGIN;
	int y = MARGIN;

	unsigned int color = IsQuotaCleared() ? 0x00ff00 : 0xffffff;
	DrawFormatString(x, y, color, "%s", buf);

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
	transform_.scl = { 2.0f, 2.0f, 2.0f };
	// 座標
	transform_.pos = { 0.0f, 0.0f, 0.0f };

	// 座標
	roketPos_ = { -405.45f, 2593.0f, 660.0f };

	transform_.Update();
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
