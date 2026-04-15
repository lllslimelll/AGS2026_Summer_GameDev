#include <string>
#include <fstream>
#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "EnemyRat.h"
#include "EnemyRobot.h"
#include "EnemyManager.h"

EnemyManager::EnemyManager(Player* player)
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

	// 種別事に敵を生成
	switch (data.type)
	{
	case EnemyBase::TYPE::RAT:
		enemy = new EnemyRat(data, player_);
		break;
	case EnemyBase::TYPE::ROBOT:
		enemy = new EnemyRobot(data, player_);
		break;
	default:
		break;
	}

	if (enemy != nullptr)
	{
		enemy->Init(); // 初期化
		enemies_.emplace_back(enemy); // 敵リストに追加
	}

	return enemy;

	//// 敵の生成
	//EnemyRat* enemyRat = new EnemyRat();
	//// 敵リストに追加
	//enemies_.emplace_back(enemyRat);
	//// 敵の初期化
	//enemyRat->Init();
}

// 衝突対象となるコライダを登録
void EnemyManager::AddHitCollider(const ColliderBase* hitCollider)
{
	for (auto& enemy : enemies_)
	{
		enemy->AddHitCollider(hitCollider);
	}
}
