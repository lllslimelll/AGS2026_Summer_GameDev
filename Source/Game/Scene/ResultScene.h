#pragma once
#include "SceneBase.h"

class ResultScene :	public SceneBase
{
public:

	// コンストラクタ・デストラクタ
	ResultScene();
	~ResultScene() override;

	// ロード
	void Load(void) override {};

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

private:

	int   totalScore_ = 0;
	float revealTimer_ = 0.0f;

	static constexpr float TIME_SCORE = 0.6f;
	static constexpr float TIME_STATUS = 1.2f;
	static constexpr float TIME_GRADE = 1.8f;
};

