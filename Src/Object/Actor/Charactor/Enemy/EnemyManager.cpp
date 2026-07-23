#include <string>
#include <fstream>
#include "../../../../Application.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../Collider/ColliderCapsule.h"
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

	// 全敵の位置更新後に一括で解決（更新順序依存を消すため）
	ResolveEnemyPairPushBacks();
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

		// 空行スキップ
		if (line.empty() || line.find_first_not_of(",\r\n ") == std::string::npos) continue;

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
	case EnemyBase::TYPE::GIANT:
		enemy = new EnemyGiant(data, player_);
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
}

// 衝突対象となるコライダを登録
void EnemyManager::AddHitCollider(const ColliderBase* hitCollider)
{
	for (auto& enemy : enemies_)
	{
		enemy->AddHitCollider(hitCollider);
	}
}

void EnemyManager::ResolveEnemyPairPushBacks(void)
{
	const size_t n = enemies_.size();
	for (size_t i = 0; i < n; ++i)
	{
		const ColliderCapsule* capA = enemies_[i]->GetBodyCapsule();
		if (capA == nullptr) continue;

		for (size_t j = i + 1; j < n; ++j)
		{
			const ColliderCapsule* capB = enemies_[j]->GetBodyCapsule();
			if (capB == nullptr) continue;

			// 円柱として XZ 平面上で重なりを見る（Y は無視）
			VECTOR centerA = capA->GetCenter();
			VECTOR centerB = capB->GetCenter();

			float dx = centerA.x - centerB.x;
			float dz = centerA.z - centerB.z;
			float distSq = dx * dx + dz * dz;

			float minDist = capA->GetRadius() + capB->GetRadius();
			if (distSq >= minDist * minDist) continue;

			float dist = sqrtf(distSq);
			if (dist < 0.0001f)
			{
				// 完全重なり：任意方向で分離
				dx = 1.0f;
				dz = 0.0f;
				dist = 1.0f;
			}

			float overlap = minDist - dist;
			float invDist = 1.0f / dist;

			// A を A→B の反対方向、B を A→B 方向へ、それぞれ半量ずつ
			VECTOR pushA = { dx * invDist * overlap * 0.5f, 0.0f, dz * invDist * overlap * 0.5f };
			VECTOR pushB = { -pushA.x, 0.0f, -pushA.z };

			enemies_[i]->ApplyPushBackXZ(pushA);
			enemies_[j]->ApplyPushBackXZ(pushB);
		}
	}
}

