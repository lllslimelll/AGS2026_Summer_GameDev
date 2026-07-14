#pragma once
#include <vector>
#include <memory>
#include <DxLib.h>
#include "SceneBase.h"

class Camera;
class StageManager;

class DebugScene : public SceneBase
{
public:

	// コンストラクタ
	DebugScene(void);
	// デストラクタ
	~DebugScene(void) override;

	// 初期化
	void Init(void) override;
	// 更新
	void Update(void) override;
	// 描画
	void Draw(void) override;

private:

	std::unique_ptr<Camera> camera_;

	// ステージ
	StageManager* stageMng_;

	// デバッグポイント群
	std::vector<VECTOR> points_;

	// デバッグポイントの配置
	void PlaceDebugPoint(void);
	// デバッグポイントの保存
	void SavePoints(void);
};

