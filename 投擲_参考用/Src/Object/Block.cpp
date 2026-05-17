#include "../Manager/BlockManager.h"
#include "Block.h"

Block::Block(void)
{
}

Block::~Block(void)
{
}

void Block::Create(TYPE type, int baseModelId, int mapX, int mapZ)
{
	// ブロックの種類
	type_ = type;

	// モデルのハンドルID
	modelId_ = MV1DuplicateModel(baseModelId);

	// 色の調整（自己発光）
	MV1SetMaterialEmiColor(modelId_, 0, COCLOR_EMI);

	// １ブロック当たりの大きさ
	const float SIZE_BLOCK = BlockManager::SIZE_BLOCK;
	// 1ブロック当たりの半分の大きさ
	const float SIZE_HALF_BLOCK = (SIZE_BLOCK / 2.0f);

	// 引数で指定されたマップ座標から座標を指定する
	// 今回の3Dモデルの中心座標は、ブロックの中心に位置する
	float x = static_cast<float>(mapX);
	float z = static_cast<float>(mapZ);
	VECTOR pos = {
		((SIZE_BLOCK * x) + SIZE_HALF_BLOCK) - BlockManager::WOELD_SIZE / 2,
		-SIZE_HALF_BLOCK,
		((SIZE_BLOCK * z) + SIZE_HALF_BLOCK) - BlockManager::WOELD_SIZE / 2
	};

	// 座標設定
	pos_ = pos;
	MV1SetPosition(modelId_, pos_);

	//大きさ設定
	scales_ = SCALES;
	MV1SetScale(modelId_, scales_);

	// 衝突判定(モデルの全て)の前準備を行う
	MV1SetupCollInfo(modelId_, -1);
}

void Block::Update(void)
{
}

void Block::Draw(void)
{
	// モデルの描画
	MV1DrawModel(modelId_);
}

void Block::Release(void)
{
	// モデルのメモリ開放
	MV1DeleteModel(modelId_);
}

int Block::GetModelId(void)
{
	return modelId_;
}
