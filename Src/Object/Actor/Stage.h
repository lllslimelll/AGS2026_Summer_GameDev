#pragma once
#include <vector>
#include <string>
#include <DxLib.h>
#include "ActorBase.h"


class Stage : public ActorBase
{
public:

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL = 0,
		MAX,
	};

	~Stage(void) override;

	// 更新
	void Update(void) override;

protected:

	// リソースロード
	virtual void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	virtual void InitTransform(void) override;

	// 衝突判定の初期化
	virtual void InitCollider(void) override;

	// アニメーションの初期化
	virtual void InitAnimation(void) override;

	// 初期化後の個別処理
	virtual void InitPost(void) override;

private:

	// 除外フレーム名称
	const std::vector<std::string> EXCLUDE_FRAME_NAMES = {
		
	};

	// 対象フレーム
	const std::vector<std::string> TARGET_FRAME_NAMES = {
		"Ground",
	};
};

