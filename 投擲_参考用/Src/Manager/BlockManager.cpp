#include <string>
#include <fstream>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Object/Block.h"
#include "BlockManager.h"

BlockManager::BlockManager(void)
{
}

BlockManager::~BlockManager(void)
{
}

void BlockManager::Init(void)
{
	// 各種ブロックモデルのロード
	std::string PATH = Application::PATH_MODEL + "Blocks/";
	baseBlockModelIds_[0] = MV1LoadModel((PATH + "Block_Grass.MV1").c_str());
	baseBlockModelIds_[1] = MV1LoadModel((PATH + "Block_Metal.MV1").c_str());
	baseBlockModelIds_[2] = MV1LoadModel((PATH + "Block_Ice.MV1").c_str());

	// csvでマップ生成
	LoadMapCsvData();
}

void BlockManager::Update(void)
{
	// マップの描画
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			if (blocks_[z][x])
			{
				blocks_[z][x]->Update();
			}
		}
	}
}

void BlockManager::Draw(void)
{
	// マップの描画
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			if (blocks_[z][x])
			{
				blocks_[z][x]->Draw();
			}
		}
	}

}

void BlockManager::Release(void)
{
	// モデルのメモリ開放
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			blocks_[z][x]->Release();
			delete blocks_[z][x];
		}
	}

	// モデルのメモリ開放
	for (int i = 0; i < NUM_BLOCK_MODELS; i++)
	{
		MV1DeleteModel(baseBlockModelIds_[i]);
	}
}

// 衝突判定
bool BlockManager::IsCollisionLine(VECTOR topPos, VECTOR downPos, MV1_COLL_RESULT_POLY* result)
{
	bool ret = false;
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			// 2次元配列からBlock情報を取り出す
			Block* block = blocks_[z][x];
			// 線分とモデルの衝突判定
			MV1_COLL_RESULT_POLY res =
				MV1CollCheck_Line(block->GetModelId(), -1, topPos, downPos);
			if (res.HitFlag)
			{
				// 結果を返す
				*result = res;
				return true;
			}
		}
	}
	return ret;
}

void BlockManager::LoadMapCsvData(void)
{
	// ファイルの読込
	std::ifstream ifs = std::ifstream(Application::PATH_MAP_DATA);
	if (!ifs)
	{
		// エラーが発生
		return;
	}
	
	// ファイルを１行ずつ読み込む
	std::string line;
	std::vector<std::string> strSplit;
	int chipNo = 0;
	int z = 0;
	while (getline(ifs, line))
	{
		// １行をカンマ区切りで分割
	   // 1行の文字情報
	   // 1行を1文字の動的配列に分割
		strSplit = AsoUtility::Split(line, ',');
		for (int x = 0; x < strSplit.size(); x++) {

			// stringからintに変換
			chipNo = stoi(strSplit[x]);
			// int型をキャストして enum classに変換
			Block::TYPE type = static_cast<Block::TYPE>(chipNo);

			// 先に読み込んでいたモデルのハンドルIDを取得する
			int baseModelId = baseBlockModelIds_[chipNo];

			// ブロックを生成
			Block* block = new Block();
			block->Create(type, baseModelId, x, z);

			// ２次元配列にブロッククラスのポインタを格納
			blocks_[z][x] = block;
		}

		z++;
	}
}
