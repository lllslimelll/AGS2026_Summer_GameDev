#pragma once
#include <vector>
#include <map>
class EnemyBase;
class Player;

class EnemyManager
{
public:

	enum class WAVE
	{
		WAVE01,
		WAVE02,
		WAVE03,
		END
	};


	// 敵のウェーブによる出現間隔
	static constexpr int SPAWN_INTERVAL_WAVE01 = 180;
	static constexpr int SPAWN_INTERVAL_WAVE02 = 240;
	static constexpr int SPAWN_INTERVAL_WAVE03 = 1;

	// 次のウェーブに進む時間
	static constexpr int CNT_NEXT_WAVE01 = 500;
	static constexpr int CNT_NEXT_WAVE02 = 500;

	// コンストラクタ
	EnemyManager(Player* player);
	// デストラクタ
	~EnemyManager(void);
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// 敵取得
	const std::map<EnemyBase::TYPE, std::vector<EnemyBase*>>& GetEnemies();

	// ウェーブ状態取得
	WAVE GetWave(void);

private:

	// ウェーブ状態
	WAVE wave_;

	// エネミー連想配列
	std::map<EnemyBase::TYPE, std::vector<EnemyBase*>> enemies_;

	// 敵用のモデルハンドルID
	std::vector<int> enemyModelIds_;
	// 攻撃エフェクト用のモデルハンドルID
	std::vector<int> attackEffectModelIds_;

	// プレイヤーのポインタ
	Player* player_;
	// 出現間隔
	float cntSpawnDemon_;
	float cntSpawnWizard_;

	// 出現間隔
	int cntSpawn_;

	// ウェーブ遷移
	void ChangeWave(WAVE wave);

	// ウェーブ別の更新処理
	void UpdateWave01(void);
	void UpdateWave02(void);
	void UpdateWave03(void);
	void UpdateWaveEnd(void);
};


