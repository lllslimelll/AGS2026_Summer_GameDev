#pragma once
#include "Scene.h"

class ResultScene :	public Scene
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

	int totalScore_ = 0;
};

