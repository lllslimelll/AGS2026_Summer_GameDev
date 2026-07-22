#pragma once
#include "PostEffect.h"

class PostEffectGuideUI : public PostEffect
{
public:

	// コンストラクタ
	PostEffectGuideUI(void);

	// デストラクタ
	~PostEffectGuideUI(void) override;

	// エフェクトの初期化
	void InitEffect(void) override;

	// 更新
	void Update() override;

private:

	// 定数バッファのサイズ
	static constexpr int CONST_BUF_FLOAT4_SIZE = 1;

	// テクスチャのスロット数
	static constexpr int TEX_SLOT_NUM = 1;

	// 歪みの強度
	float distortion_;
};

