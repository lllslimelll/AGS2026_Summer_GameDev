#pragma once
#include <vector>
#include "EnemyBase.h"

class Player;

class EnemyManager
{
public:

	// コンストラクタ
	EnemyManager(Player& player);
	// デストラクタ
	~EnemyManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// CSV読み込み
	void LoadCsvData(void);

	// エネミー生成
	EnemyBase* Create(const EnemyBase::EnemyData& data);

	// エネミー取得
	const std::vector<EnemyBase*>& GetEnemies(void) const { return enemies_; }

private:

	// プレイヤー
	Player& player_;

	// エネミーリスト
	std::vector<EnemyBase*> enemies_;
};

