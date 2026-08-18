#pragma once
#include "SceneBase.h"

class ResultScene :	public SceneBase
{
public:

	// コンストラクタ・デストラクタ
	ResultScene();
	~ResultScene() override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

private:

	int   totalScore_ = 0;
	float revealTimer_ = 0.0f;

	bool playedScore_ = false;
	bool playedStatus_ = false;
	bool playedGrade_ = false;

	static constexpr float TIME_SCORE = 1.0f;
	static constexpr float TIME_STATUS = 2.0f;
	static constexpr float TIME_GRADE = 3.0f;
};

