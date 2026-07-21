#include <string>
#include <fstream>
#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "EnemyGiant.h"
#include "EnemyManager.h"

EnemyManager::EnemyManager(Player& player)
	: 
	player_(player)
{
}

EnemyManager::~EnemyManager(void)
{
}

void EnemyManager::Init(void)
{
	// エネミーのデータ読み込み
	LoadCsvData();
}

// 更新
void EnemyManager::Update(void)
{
	for (auto& enemy : enemies_)
	{
		enemy->Update();
	}
}

// 描画
void EnemyManager::Draw(void)
{
	for (auto& enemy : enemies_)
	{
		enemy->Draw();
	}
}

// 解放
void EnemyManager::Release(void)
{
	for (auto& enemy : enemies_)
	{
		enemy->Release();
		delete enemy;
	}
	enemies_.clear();
}

void EnemyManager::LoadCsvData(void)
{
	// ファイルの読み込み
	std::ifstream ifs = std::ifstream(Application::PATH_CSV + "EnemyData.csv");
	// エラーが発生
	if (!ifs) return;

	// ファイルを1行ずつ読み込む
	std::string line; // 1行の文字情報
	std::vector<std::string> strSplit; // 1行を1文字の動的配列に分割

	bool isHeader = true;

	while (getline(ifs, line))
	{
		if (isHeader)
		{
			isHeader = false;
			continue;
		}

		// １行をカンマ区切りで分割
		strSplit = AsoUtility::Split(line, ',');

		EnemyBase* enemy = nullptr;

		// 構造体に合わせて読み込みデータを格納
		EnemyBase::EnemyData data = EnemyBase::EnemyData();
		int idx = 0;
		// ID
		data.id = stoi(strSplit[idx++]);
		// 種別
		data.type = static_cast<EnemyBase::TYPE>(stoi(strSplit[idx++]));
		// HP
		data.hp = stoi(strSplit[idx++]);
		// 初期座標
		data.defaultPos =
		{
			stof(strSplit[idx++]),
			stof(strSplit[idx++]),
			stof(strSplit[idx++]),
		};
		// 移動可能範囲
		data.movableRange = stof(strSplit[idx++]);
		// エネミー生成
		Create(data);
	}
	
	ifs.close();
}

EnemyBase* EnemyManager::Create(const EnemyBase::EnemyData& data)
{
	EnemyBase* enemy = nullptr;

	switch (data.type)
	{
	case EnemyBase::TYPE::GIANT:
		enemy = new EnemyGiant(data, player_);
		break;
	default: break;
	}

	if (enemy)
	{
		enemy->Init();
		enemies_.emplace_back(enemy);
	}
	return enemy;
}