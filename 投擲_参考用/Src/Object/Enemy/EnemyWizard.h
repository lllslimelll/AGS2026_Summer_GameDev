#pragma once
#include "EnemyBase.h"

class EnemyWizard : public EnemyBase
{
public:
	// 攻撃間隔
	static constexpr int TERM_ATTACK = 120;

	// コンストラクタ
	EnemyWizard(void);
	// デストラクタ
	~EnemyWizard(void); 

protected:
	// パラメータ設定
	void SetParam() override;

	// 待機状態
	void UpdateStandby() override;
};

