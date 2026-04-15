#pragma once
#include <vector>
#include "EnemyBase.h"
#include "../../../../Object/Collider/ColliderBase.h"
class ColliderBase;
class Player;

class EnemyManager
{
public:

	// コンストラクタ
	EnemyManager(Player* player);
	// デストラクタ
	~EnemyManager(void);

	// 初期化
	void Init(void);
	// 更新
	void Update(void);
	// 描画
	void Draw(void);
	// 解放
	void Release(void);

	// CSV読み込み
	void LoadCsvData(void);

	// エネミー生成
	EnemyBase* Create(const EnemyBase::EnemyData& data);

	// エネミー取得
	const std::vector<EnemyBase*>& GetEnemies(void) const { return enemies_; }

	// 衝突対象となるコライダを登録
	void AddHitCollider(const ColliderBase* hitCollider);

private:

	// プレイヤー
	Player* player_;

	// エネミーリスト
	std::vector<EnemyBase*> enemies_;
};

