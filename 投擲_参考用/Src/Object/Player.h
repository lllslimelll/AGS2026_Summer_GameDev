#pragma once
#include <DxLib.h>
class GameScene;
class AnimationController;
class WeaponBase;
class WeaponPunch;
class WeaponAxe;

class Player
{
public:

	// 状態
	enum class STATE
	{
		NONE,
		STANDBY,
		KNOCKBACK,
		ATTACK,
		DEAD,
		END,
		VICTORY,
	};


	// アニメージョン識別
	enum class ANIM_TYPE
	{
		DEATH,
		Duck,
		HIT_REACT,
		IDLE,
		IDLE_ATTACK,
		IDLE_HOLD,
		JUMP,
		JUMP_IDLE,
		JUMP_LAND,
		NO,
		PUNCH,
		RUN,
		RUN_ATTACK,
		RUN_HOLD,
		WALK,
		WALK_HOLD,
		WAVE,
		YES,
		MAX,
	};

	// モデルの大きさ
	static constexpr VECTOR SCALES = { 0.6f, 0.6f, 0.6f };
	// 初期位置
	static constexpr VECTOR DEFAULT_POS = { 0.0f, 0.0f, 0.0f };
	// 標準の自己発光色
	static constexpr COLOR_F COLOR_EMI_DEFAULT = { 0.5f, 0.5f, 0.5f, 0.5f };
	
	// 移動スピード
	static constexpr float SPEED_MOVE = 10.0f;

	// 重力
	static constexpr float GRAVITY_POW = 0.8f;

	// ジャンプ力
	static constexpr float JUMP_POW = 20.0f;


	// 地面衝突用線分の長さ
	static constexpr float COLLISION_LEN = 10.0f;

	// リスポーン開始位置
	static constexpr float RESPAWN_LEN = -1500.0f;

	// リスポーン位置
	static constexpr VECTOR RESPAWN_POS = { 0.0f, 500.0f, 0.0f };

	// 当たり判定用半径
	static constexpr float COLLISION_RADIUS = 50.0f;

	// ノックバックスピード
	static constexpr float SPEED_KNOCKBACK = 10.0f;

	// 標準の拡散光色
	static constexpr COLOR_F COLOR_DIF_DEFAULT = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 点滅時の拡散光色
	static constexpr COLOR_F COLOR_DIF_BLINK = { 1.0f, 0.5f, 0.5f, 1.0f };
	// 点滅間隔
	static constexpr int TERM_BLINK = 8;

	// 最大HP
	static constexpr int MAX_HP = 5;

	// コンストラクタ
	Player(void);
	// デストラクタ
	~Player(void);
	void Init(GameScene* gameScene);
	void Update(void);
	void Draw(void);
	void Release(void);

	// 衝突判定
	void CollisionStage(VECTOR pos);

	// リスポーン処理
	void Respawn();
	
	// 状態遷移
	void ChangeState(STATE state);

	// 指定方向にノックバックさせる
	void KnockBack(VECTOR dirXZ, float jumpPow);

	// 無敵中(ダメージ、ノックバックを受けない)
	bool IsInvincible(void);

	// 終了遷移
	bool IsStateEnd(void);

	// ダメージを与える
	void Damage(int damage);
	// HPの取得
	int GetHp(void);


	VECTOR GetPos(void) const;
	void SetPos(VECTOR pos);

	WeaponBase* GetUseWeapon(void) const;

	// 勝利状態へ移行
	void Victory(void);

private:

	GameScene* gameScene_;
	AnimationController* animationController_;

	// 状態
	STATE state_;

	// 武器
	WeaponBase* useWeapon_;
	WeaponPunch* weaponPunch_;
	WeaponAxe* weaponAxe_;

	// モデルのハンドルID
	int modelId_;
	VECTOR pos_;
	VECTOR angles_;
	VECTOR scales_;

	// 移動方向
	VECTOR moveDir_;

	// ジャンプ判定
	bool isJump_;
	// ジャンプ力
	float jumpPow_;

	// ノックバック方向
	VECTOR knockBackDir_;
	// ノックバックカウンタ(点滅用)
	int cntKnockBack_;

	// HP
	int hp_;

	// 行動制御
	void ProcessMove(void);
	void ProcessJump(void);
	void ProcessAttack(void);

	// 状態遷移
	void ChangeStanby(void);
	void ChangeKnockBack(void);
	void ChangeAttack(void);
	void ChangeDead(void);
	void ChangeEnd(void);
	void ChangeVictory(void);

	// 状態別更新
	void UpdateStanby(void);
	void UpdateKnockBack(void);
	void UpdateAttack(void);
	void UpdateDead(void);
	void UpdateEnd(void);
	void UpdateVictory(void);
	// 状態別描画
	void DrawStanby(void);
	void DrawKnockBack(void);
	void DrawAttack(void);
	void DrawDead(void);
	void DrawEnd(void);
	void DrawVictory(void);
};

