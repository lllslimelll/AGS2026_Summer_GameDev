#pragma once
#include "../Object/Common/Transform.h"
#include "SceneBase.h"
class AnimationController;
class SkyDome;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:
	SkyDome* skyDome_;

	int imgTitle_;
	int imgPushSpace_;

	// 惑星
	Transform bigPlanet_;

	// 球体惑星
	Transform spherePlanet_;

	// プレイヤー
	Transform player_;

	// アニメーション
	AnimationController* animController_;

	// スカイドーム用の空Transform
	Transform empty_;
};
