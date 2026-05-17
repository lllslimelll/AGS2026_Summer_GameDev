#pragma once
#include <DxLib.h>
class ShotBase
{
public:
	// 弾の種類
	enum class TYPE
	{
		STRAIGHT,
		ROCK
	};

	// コンストラクタ(元となるモデルのハンドルID)
	ShotBase(TYPE type, int baseModelId);
	// デストラクタ
	virtual ~ShotBase(void);
	// 弾の生成(表示開始座標、弾の進行方向)
	virtual void CreateShot(VECTOR pos, VECTOR dir);
	// 更新ステップ
	virtual void Update(void);
	// 描画
	void Draw(void);
	// 解放処理
	void Release(void);
	// 生存判定
	bool IsAlive(void);
	// 弾の座標
	VECTOR GetPos(void);
	// 弾の衝突判定用半径
	float GetCollisionRadius(void);
	// 弾の種別
	TYPE GetType(void);

protected:

	// 弾の種別
	TYPE type_;
	// 弾のモデルID
	int modelId_;
	// 方向
	VECTOR dir_;
	// 弾の大きさ
	VECTOR scales_;
	// 弾の座標
	VECTOR pos_;
	// 弾の移動速度
	float speed_;
	// 弾の生存判定
	bool isAlive_;
	// 弾の生存期間
	int cntAlive_;
	// 弾の衝突判定用半径
	float collisionRadius_;
	// パラメータ設定
	virtual void SetParam(void) = 0;
	// 弾の生存期間の減少
	void ReduceCntAlive(void);
};

