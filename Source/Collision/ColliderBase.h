#pragma once
#include <DxLib.h>
#include "CollisionProfile.h"
#include "CollisionProfileType.h"

class ActorBase;
class Transform;
class HitResult;

class ColliderBase
{
public:

	// 形状
	enum class SHAPE
	{
		NONE,
		LINE,
		SPHERE,
		CAPSULE,
		MODEL,
	};

	// コンストラクタ
	//  shape  : 形状
	//  type   : プロファイルの種別
	//  owner  : このコライダーを持つ Actor
	ColliderBase(
		SHAPE shape,
		CollisionProfileType type,
		ActorBase* owner);

	// デストラクタ
	virtual ~ColliderBase(void);

	// 描画
	void Draw(void);

	// 形状の取得
	SHAPE GetShape(void) const { return shape_; }

	// プロファイルの取得
	const CollisionProfile& GetProfile(void) const { return profile_; }

	// チャンネルの取得
	CollisionChannel GetChannel(void) const { return profile_.channel_; }

	// 所有者 Acotr の取得
	ActorBase* GetOwner(void) const { return owner_; }

	// 有効フラグ
	bool IsValid(void) const { return isValid_; }
	void SetValid(bool valid) { isValid_ = valid; }

	// 追従先の再設定
	void SetFollow(Transform* follow);

	// 押し戻し後の座標を取得
	virtual VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitPoly,
		int maxTryCnt,
		float pushDistance) const = 0;

protected:

	// デバッグ表示の色
	static constexpr int COLOR_VALID = 0xff0000;
	static constexpr int COLOR_INVALID = 0xaaaaaa;

	// 形状
	SHAPE shape_;
	// プロファイル
	CollisionProfile profile_;
	// オーナー
	ActorBase* owner_;
	// 有効フラグ
	bool isValid_ = true;

	// owner_ から Transform を取得してワールド座標に変換
	VECTOR GetRotPos(const VECTOR& localPos) const;

	virtual void DrawDebug(int color) = 0;
};
