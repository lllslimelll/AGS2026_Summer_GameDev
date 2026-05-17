#include <Dxlib.h>
#include "../Application.h"
#include "../Object/Shot/ShotBase.h"
#include "../Object/Enemy/EnemyBase.h"
#include "../Object/Enemy/EnemyDemon.h"
#include "../Object/Enemy/EnemyWizard.h"
#include "../Manager/SceneManager.h"
#include "EnemyManager.h"

EnemyManager::EnemyManager(Player* player)
{
	player_ = player;
}

EnemyManager::~EnemyManager(void)
{
}

void EnemyManager::Init(void)
{
	// エネミーモデルのロード
	enemyModelIds_.emplace_back(
		MV1LoadModel(("Data/Model/Enemy/Demon.mv1")));
	enemyModelIds_.emplace_back(
		MV1LoadModel(("Data/Model/Enemy/Wizard.mv1")));
	enemyModelIds_.emplace_back(
		MV1LoadModel(("Data/Model/Enemy/Giant.mv1")));

	// 攻撃エフェクト用のモデルのロード
	attackEffectModelIds_.emplace_back(
		MV1LoadModel(
			(Application::PATH_MODEL + "Effect/Fireball/Fireball.mv1").c_str()));
	attackEffectModelIds_.emplace_back(
		MV1LoadModel(
			(Application::PATH_MODEL + "Effect/Fireball/Rock.mv1").c_str()));

	// 出現カウンタ初期化
	cntSpawn_ = 0;

	// 初期ウェーブ
	ChangeWave(WAVE::WAVE01);

	// 敵の初期化
	enemies_.clear();
}

void EnemyManager::Update(void)
{
	// 出現カウンタ
	cntSpawn_++;

	switch (wave_)
	{
	case WAVE::WAVE01:
		UpdateWave01();
		break;
	case WAVE::WAVE02:
		UpdateWave02();
		break;
	case WAVE::WAVE03:
		UpdateWave03();
		break;
	case WAVE::END:
		UpdateWaveEnd();
		break;
	}


	// 登録されているすべての敵を更新
	//for (int i = 0; i < enemys_.size(); i++)
	//{
	//	if (enemys_[i] != nullptr)
	//	{
	//		enemys_[i]->Update();  // 敵の更新
	//	}
	//}

	 // すべての敵を更新
	for (const auto pair : enemies_)
	{
		for (EnemyBase* enemy : pair.second)
		{
			enemy->Update();
		}
	}
}

void EnemyManager::Draw(void)
{
	for (const auto pair : enemies_)
	{
		for (EnemyBase* enemy : pair.second)
		{
			enemy->Draw();
		}
	}
}

void EnemyManager::Release(void)
{
	for (const auto pair : enemies_)
	{
		for (EnemyBase* enemy : pair.second)
		{
			enemy->Release();
			delete enemy;
		}
	}

	// ベースとなるモデルのメモリ解放
	for (int id : enemyModelIds_)
	{
		MV1DeleteModel(id);
	}
}

void EnemyManager::ChangeWave(WAVE wave)
{
	wave_ = wave;

	// 出現カウンタをリセット
	cntSpawn_ = 0;

	switch (wave_)
	{
	case EnemyManager::WAVE::WAVE01:
		break;
	case EnemyManager::WAVE::WAVE02:
		break;
	case EnemyManager::WAVE::WAVE03:
		break;
	default:
		break;
	}
}
void EnemyManager::UpdateWave01(void)
{

	// 出現時間を増やす
	cntSpawnDemon_ += SceneManager::GetInstance().GetDeltaTime();
	cntSpawnWizard_ += SceneManager::GetInstance().GetDeltaTime();

	// 出現時間が経過したらDemon出現
	if (cntSpawn_ % SPAWN_INTERVAL_WAVE01 == 0)
	{
		// 出現時間をリセット
		cntSpawnDemon_ = 0;

		// 敵を生成
		EnemyBase* demon = new EnemyDemon();

		// 初期化
		demon->Init(EnemyBase::TYPE::DEMON,
			enemyModelIds_[static_cast<int>(EnemyBase::TYPE::DEMON)],
			-1,
			player_);

		// 敵を登録
		enemies_[EnemyBase::TYPE::DEMON].emplace_back(demon);
	}

	// 次のウェイブに進む
	if (cntSpawn_ > CNT_NEXT_WAVE01)
	{
		ChangeWave(WAVE::WAVE02);
	}
}

void EnemyManager::UpdateWave02(void)
{
	// 一定間隔でWizard出現
	if (cntSpawn_ % SPAWN_INTERVAL_WAVE01 == 0)
	{
		cntSpawnWizard_ = 0;

		EnemyBase* wizard = new EnemyWizard();

		wizard->Init(EnemyBase::TYPE::WIZARD,
			enemyModelIds_[static_cast<int>(EnemyBase::TYPE::WIZARD)],
			attackEffectModelIds_[static_cast<int>(ShotBase::TYPE::STRAIGHT)],
			player_);

		enemies_[EnemyBase::TYPE::WIZARD].emplace_back(wizard);
	}

	// 次のウェイブに進む
	if (cntSpawn_ > CNT_NEXT_WAVE02)
	{
		ChangeWave(WAVE::WAVE03);
	}
}

void EnemyManager::UpdateWave03(void)
{
	// 敵の全滅判定
	bool isAllDead = true;

	for (const auto pair : enemies_)
	{
		for (EnemyBase* enemy : pair.second)
		{
			if (enemy->IsAlive())
			{
				isAllDead = false; // 1体でも生存していれば全滅ではない
				break; // ループを抜ける
			}
		}
	}

	// 全滅ならウェーブ終了へ
	if (isAllDead)
	{
		ChangeWave(WAVE::END);
	}
}

void EnemyManager::UpdateWaveEnd(void)
{
}

const std::map<EnemyBase::TYPE, std::vector<EnemyBase*>>& EnemyManager::GetEnemies()
{
	return enemies_;
}

EnemyManager::WAVE EnemyManager::GetWave(void)
{
	return wave_;
}

