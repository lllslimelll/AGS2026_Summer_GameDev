#pragma once
#include <vector>
#include <DxLib.h>
class Player;
class AnimationController;
class ShotBase;


class EnemyBase
{
public:

	// エネミー種別
	enum class TYPE
	{
		DEMON,
		WIZARD,
		GIANT,
		MAX,
	};

	enum class ANIM_TYPE
	{
		ATTACK,
		DEATH,
		HIT_REACT,
		IDLE,
		JUMP,
		RUN,
		WALK,
		MAX,
	};

	// 状態
	enum class STATE
	{
		NONE,
		STANDBY,
		ATTACK,
		HIT_REACT,
		DEAD_REACT,
		END
	};


	// モデルの大きさ
	static constexpr VECTOR DEFAULT_SCALES = { 1.0f, 1.0f, 1.0f };
	// 初期位置
	static constexpr VECTOR DEFAULT_POS = { 200.0f, 0.0f, 800.0f };
	// 敵初期角度
	static constexpr VECTOR DEFAULT_ANGLE = { 0.0f, 0.0f, 0.0f };

	// 移動スピード
	static constexpr float MOVE_SPEED = 5.0f;

	// 標準の自己発光色
	static constexpr COLOR_F COLOR_EMI_DEFAULT = { 0.6f, 0.6f, 0.6f, 1.0f };


	// 攻撃エフェクトのモデルID// 標準の拡散光色
	static constexpr COLOR_F COLOR_DIF_DEFAULT = { 0.4f, 0.4f, 0.4f, 1.0f };
	// 点滅時の拡散光色
	static constexpr COLOR_F COLOR_DIF_BLINK = { 1.0f, 0.2f, 0.2f, 1.0f };
	// 点滅間隔
	static constexpr int TERM_BLINK = 8;
	// 被ダメ時間
	static constexpr int CNT_HIT_REACT = 40;
	// 死亡時間
	static constexpr int CNT_DEAD_REACT = 80;

	// コンストラクタ
	EnemyBase(void);
	// デストラクタ
	virtual ~EnemyBase(void);
	void Init(TYPE type,
		int basemodelId, int baseAttackEffectModelId, Player* player);
	void Update(void);
	void Draw(void);
	void Release(void);

	// 追従対象を向く処理
	void LookPlayer();

	// 移動処理
	void Move();

	// 出現位置
	void SetSpawnPos();

	// 状態遷移
	void ChangeState(STATE state);

	VECTOR GetPos(void);

	bool GetAlive(void);

	void Damage(int damage);

	// 衝突判定が有効な状態
	bool IsCollisionState(void);

	// 生存判定
	bool IsAlive(void);


	// 衝突判定用半径を取得  
	virtual float GetCollisionRadius(void);

	const std::vector<ShotBase*>& GetShots(void);

protected:
	// 敵の追従対象
	Player* player_;

	// アニメーション
	AnimationController* animationController_;

	// 種別
	TYPE type_;

	// 状態
	STATE state_;

	int modelId_;

	VECTOR pos_;
	VECTOR angle_;
	VECTOR scales_;
	
	VECTOR moveDir_;
	float dist_;

	float speed_;
	// ステージの半径
	float radius_;

	// 攻撃
	int cntAttack_;

	// 衝突判定用半径
	float collisionRadius_;

	// 被ダメカウンタ(点滅演出にも使用)
	int cntDamaged_;

	// 弾
	std::vector<ShotBase*> shots_;
	// エフェクト用モデルハンドルID
	int baseAttackEffectModelId_;

	// HP
	int hp_;

	// 弾の更新
	void UpdateShot(void);
	// 弾の描画
	void DrawShot(void);
	// 有効な弾を取得
	ShotBase* GetValidShot(void);

	// パラメータ設定(純粋仮想関数)
	virtual void SetParam() = 0;

	// 状態遷移
	virtual void ChangeStandby(void);
	virtual void ChangeAttack(void);
	virtual void ChangeHitReact(void);
	virtual void ChangeDeadReact(void);
	virtual void ChangeEnd(void);
	// 状態別更新
	virtual void UpdateStandby(void);
	virtual void UpdateAttack(void);
	virtual void UpdateHitReact(void);
	virtual void UpdateDeadReact(void);
	virtual void UpdateEnd(void);
	// 状態別描画
	virtual void DrawStandby(void);
	virtual void DrawAttack(void);
	virtual void DrawHitReact(void);
	virtual void DrawDeadReact(void);
	virtual void DrawEnd(void);
};

