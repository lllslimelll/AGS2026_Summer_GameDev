#include <DxLib.h>
#include "../Object/Camera.h"
#include "../Common/Grid.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/BlockManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Object/Player.h"
#include "../Object/Enemy/EnemyBase.h"
#include "../Object/Shot/ShotBase.h"
#include "../Object/Weapon/WeaponBase.h"
#include "../Manager/EnemyManager.h"
#include "../UI/HpManager.h"
#include "../Application.h"
#include "GameScene.h"



void GameScene::Init(void)
{
	// グリッド
	grid_ = new Grid();
	blockManager_ = new BlockManager();
	player_ = new Player();
	enemyManager_ = new EnemyManager(player_);
	hpManager_ = new HpManager(player_);

	grid_->Init();

	blockManager_->Init();

	player_->Init(this);

	enemyManager_->Init();

	hpManager_->Init();

	// ゲームおーば判定
	isGameOver_ = false;

	// ゲームオーバー画像
	imgGameOver_ = LoadGraph(
		(Application::PATH_IMAGE + "Gameover.png").c_str());

	// ゲームクリア画像
	imgGameClear_ = LoadGraph(
		(Application::PATH_IMAGE + "GameClear.png").c_str());

	// パンチアイコン
	imgWeaponIconPunch_ = LoadGraph(
		(Application::PATH_IMAGE + "Punch.png").c_str());

	// 斧アイコン
	imgWeaponIconAxe_ = LoadGraph(
		(Application::PATH_IMAGE + "Axe.png").c_str());
}

void GameScene::Update(void)
{
	// ゲームオーバー判定
	if (player_->IsStateEnd()) {
		isGameOver_ = true;
	}
	// ゲームクリア判定
	if (!isGameClear_ && enemyManager_->GetWave() == EnemyManager::WAVE::END) {

		isGameClear_ = true;
	
		player_->Victory();
	}

	if (isGameOver_ || isGameClear_) 
	{
		InputManager& ins = InputManager::GetInstance();
		if (ins.IsTrgDown(KEY_INPUT_SPACE)) {
			SceneManager::GetInstance().ChangeScene(
				SceneManager::SCENE_ID::TITLE);
		}

		// プレイヤーのアニメーションは続行される
		player_->Update();

		// 処理中断
		return;
	}

	//camera_->Update();
	blockManager_->Update();
	
	player_->Update();

	enemyManager_->Update();

	// 衝突判定
	Collision();

	// 敵とプレイヤーの当たり判定
	CollisionEnemy();

	// 弾とプレイヤーの当たり判定
	CollisionEShot();

	// 敵とプレイヤーの武器の衝突
	CollisionWeapon();
}

void GameScene::Draw(void)
{
	//camera_->DrawDebug();
	blockManager_->Draw();

	//grid_->Draw();

	player_->Draw();

	enemyManager_->Draw();

	hpManager_->Draw();

	//使用武器アイコン
	WeaponBase::TYPE type = player_->GetUseWeapon()->GetType();
	int icon = -1;

	switch (type)
	{
	case WeaponBase::TYPE::PUNCH:
		icon = imgWeaponIconPunch_;
		break;
	case WeaponBase::TYPE::AXE:
		icon = imgWeaponIconAxe_;
		break;
	}

	DrawRotaGraph(
		USE_WEAPON_ICON_POS_X,
		USE_WEAPON_ICON_POS_Y,
		1.0, 0.0, icon, true);

	if (isGameOver_) {
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
					Application::SCREEN_SIZE_Y / 2,
					1.0, 0.0, imgGameOver_, true);
	}

	if (isGameClear_) {
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2,
			Application::SCREEN_SIZE_Y / 2,
			1.0, 0.0, imgGameClear_, true);
	}

#ifdef DEBUG
	//// 地面との衝突用線分
	//DrawSphere3D(lineTopPos_, 20.0f, 10, 0x00ff00, 0x00ff00, true);
	//DrawSphere3D(lineDownPos_, 20.0f, 10, 0x00ff00, 0x00ff00, true);
	//DrawLine3D(lineTopPos_, lineDownPos_, 0xff0000);
#endif // DEBUG
}

void GameScene::Release(void)
{
	blockManager_->Release();
	delete blockManager_;

	player_->Release();
	delete player_;

	enemyManager_->Release();
	delete enemyManager_;

	hpManager_->Release();
	delete hpManager_;

	DeleteGraph(imgGameOver_);
	DeleteGraph(imgGameClear_);

	DeleteGraph(imgWeaponIconPunch_);
	DeleteGraph(imgWeaponIconAxe_);
}

void GameScene::Collision(void)
{
	// ステージブロックとプレイヤーの衝突
	CollisionStage();
}

void GameScene::CollisionStage(void)
{
	// ステージブロックとプレイヤーの衝突
	VECTOR playerPos = player_->GetPos();
	// 線分の上座標
	VECTOR topPos = playerPos;
	topPos.y = playerPos.y + (Player::COLLISION_LEN * 2.0f);
	lineTopPos_ = topPos;
	// 線分の下座標
	VECTOR downPos = playerPos;
	downPos.y = playerPos.y - Player::COLLISION_LEN;
	lineDownPos_ = downPos;
	// 線分とブロックモデルの衝突判定
	MV1_COLL_RESULT_POLY result;
	if (blockManager_->IsCollisionLine(topPos, downPos, &result))
	{
		// プレイヤーに衝突座標を渡す
		player_->CollisionStage(result.HitPosition);
	}

}

// エネミーとプレイヤーの衝突判定
void GameScene::CollisionEnemy(void)
{
	VECTOR pPos = player_->GetPos();

	// 敵の情報を取得
	auto& enemies = enemyManager_->GetEnemies();
	// 連想配列（map）を for で回す
	for (auto pair : enemies)
	{
		for (EnemyBase* enemy : pair.second)
		{
			// プレイヤーが無敵だったら早期リターン
			if (player_->IsInvincible()) {
				return;
			}

			if (!enemy->IsCollisionState())
			{
				// 衝突が有効状態でなければ、衝突判定しない
				continue;
			}

			// 敵座標
			VECTOR ePos = enemy->GetPos();

			// 半径の合計
			float pRadius = Player::COLLISION_RADIUS;
			float eRadius = enemy->GetCollisionRadius();

			// 球体同士の当たり判定
			if (AsoUtility::IsHitSpheres(pPos, pRadius, ePos, eRadius))
			{
				// 相手へのベクトルを計算（引き算）
				VECTOR diff = VSub(pPos, ePos);
				diff.y = 0.0f;

				// ベクトルの正規化で単位ベクトル（方向）を取得
				VECTOR dir = VNorm(diff);

				// プレイヤーをノックバックさせる
				player_->KnockBack(dir, 20.0);

				// ダメージを与える
				player_->Damage(1);
			}
		}
	}

}

void GameScene::CollisionEShot(void)
{
	VECTOR pPos = player_->GetPos();

	// 敵の情報を取得
	auto& enemies = enemyManager_->GetEnemies();
	// 連想配列（map）を for で回す
	for (auto pair : enemies)
	{
		for (EnemyBase* enemy : pair.second)
		{
			// プレイヤーが無敵だったら早期リターン
			if (player_->IsInvincible()) {
				return;
			}

			// 弾取得
			auto& shots = enemy->GetShots();

			// 弾配列を回す
			for (ShotBase* shot : shots)
			{
				// 弾取得
				VECTOR eShot = shot->GetPos();

				// プレイヤーと弾の当たり判定用半径
				float pRadius = Player::COLLISION_RADIUS;
				float shotRadius = shot->GetCollisionRadius();

				// 弾とプレイヤーの当たり判定
				if (AsoUtility::IsHitSpheres(pPos, pRadius, eShot, shotRadius))
				{
					// 相手へのベクトルを計算（引き算）
					VECTOR diff = VSub(pPos, eShot);
					diff.y = 0.0f;

					// ベクトルの正規化で単位ベクトル（方向）を取得
					VECTOR dir = VNorm(diff);

					// プレイヤーをノックバックさせる
					player_->KnockBack(dir, 20.0);

					// ダメージを与える
					player_->Damage(1);
				}
			}
		}
	}
}

// 敵とプレイヤーの武器の衝突
void GameScene::CollisionWeapon(void)
{
	// プレイヤーの武器を取得
	WeaponBase* useWeapon = player_->GetUseWeapon();

	// 武器が消滅していたら
	if (!useWeapon->IsAlive()) {
		return;
	}

	// 武器の衝突判定
	VECTOR weaponPos = useWeapon->GetPos();
	float weaponRadius = useWeapon->GetCollisionRadius();

	// 敵の情報を取得
	auto& enemies = enemyManager_->GetEnemies();
	// 連想配列（map）を for で回す
	for (auto pair : enemies){
		for (EnemyBase* enemy : pair.second){

			if (!enemy->IsCollisionState())
			{
				// 衝突が有効状態でなければ、衝突判定しない
				continue;
			}

			// 敵座標
			VECTOR ePos = enemy->GetPos();
			float eRadius = enemy->GetCollisionRadius();

			// 球体と球体の衝突判定
			if (AsoUtility::IsHitSpheres(
				weaponPos, weaponRadius,
				ePos, eRadius))
			{
				// 敵にダメージを与える
				enemy->Damage(1);
			}

		}
	}
}

EnemyBase* GameScene::GetEnemyBase(void) const
{
	return enemyBase_;
}
