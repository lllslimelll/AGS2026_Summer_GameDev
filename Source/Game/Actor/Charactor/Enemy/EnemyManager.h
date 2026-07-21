#pragma once
#include <vector>
#include <functional>
#include "../../../../Game/Actor/ActorBase.h"
#include "EnemyBase.h"

class Player;

class EnemyManager : public ActorBase
{
public:

	// Enemy 生成を GameScene(World 相当)に委譲するための関数オブジェクト
	using SpawnEnemyFunc = std::function<EnemyBase* (const EnemyBase::EnemyData&)>;

	// コンストラクタ
	EnemyManager(Player& player, SpawnEnemyFunc spawnFunc);
	// デストラクタ
	~EnemyManager(void);

	void Init(void)    override;
	void Update(void)  override;
	void Draw(void)    override;
	void Release(void) override;

	// CSV読み込み
	void LoadCsvData(void);

	// エネミー生成
	EnemyBase* Create(const EnemyBase::EnemyData& data);

	// エネミー取得
	const std::vector<EnemyBase*>& GetEnemies(void) const { return enemies_; }

private:

	// プレイヤー
	Player& player_;

	SpawnEnemyFunc spawnFunc_;

	// エネミーリスト
	std::vector<EnemyBase*> enemies_;
};

