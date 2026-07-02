#pragma once
#include <vector>
#include <memory>
#include <DxLib.h>
#include "Scene.h"
<<<<<<< HEAD
=======

class Camera;
>>>>>>> c43593c588c266bd4e7c5e84d0a77fe0702bb34e
class Stage;

class DebugScene : public Scene
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
	Stage* stage_;

	// デバッグポイント群
	std::vector<VECTOR> points_;

	// デバッグポイントの配置
	void PlaceDebugPoint(void);
	// デバッグポイントの保存
	void SavePoints(void);
};

