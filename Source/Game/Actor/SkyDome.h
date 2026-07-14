#pragma once
#include <DxLib.h>
#include "ActorBase.h"


class SkyDome : public ActorBase
{

public:

	// 状態
	enum class STATE
	{
		NONE,
		STAY,
		FOLLOW
	};

	SkyDome(const Transform& followTransform);

	~SkyDome() override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

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

	// 追従対象のTransform
	const Transform& followTransform_;

	// 状態
	STATE state_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStateStay(void);
	void ChangeStateFollow(void);

	// 更新
	void UpdateNone(void);
	void UpdateStay(void);
	void UpdateFollow(void);
};