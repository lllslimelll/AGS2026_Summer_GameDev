#include <DxLib.h>
#include "../Scene/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Camera/Camera.h"
#include "../Object/Actor/ActorBase.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy/EnemyManager.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Actor/Item/ItemManager.h"
#include "../Object/Actor/SkyDome.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	stage_(nullptr),
	itemMng_(nullptr),
	player_(nullptr),
	enemyManager_(nullptr),
	skyDome_(nullptr),
	isPaused_(false),
	pauseMenuIndex_(0),
	Scene()
{
}

GameScene::~GameScene(void)
{
	stage_->Release();
	delete stage_;

	itemMng_->Release();
	delete itemMng_;

	player_->Release();
	delete player_;

	enemyManager_->Release();
	delete enemyManager_;

	skyDome_->Release();
	delete skyDome_;
}

void GameScene::Init(void)
{
	stage_ = new Stage();

	itemMng_ = new ItemManager();

	player_ = new Player(itemMng_, stage_);

	enemyManager_ = new EnemyManager(player_);

	skyDome_ = new SkyDome(player_->GetTransform());

	// 初期化
	stage_->Init();
	// ステージモデルのコライダー
	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	// アイテム
	itemMng_->Init();
	itemMng_->AddHitCollider(stageCollider);

	player_->Init();
	player_->AddHitCollider(stageCollider);	// ステージモデルのコライダー登録

	enemyManager_->Init();
	enemyManager_->AddHitCollider(stageCollider); // ステージモデルのコライダー登録
	enemyManager_->AddHitCollider(				  // キャラモデルのコライダー登録
		player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE)));

	skyDome_->Init();
	
	Camera* camera = sceMng_.GetCamera();
	camera->SetFollow(&player_->GetTransform());
	camera->ChangeMode(Camera::MODE::FOLLOW);
	camera->AddHitCollider(stageCollider); // ステージモデルのコライダー登録
	camera->SetInputEnabled(true);
}

void GameScene::Update(void)
{
	SetMouseDispFlag(false);
	auto& ins = InputManager::GetInstance();

	if (ins.IsTriggerd("pause"))
	{
		// ポーズシーンを追加
		sceMng_.PushScene(SceneManager::SCENE_ID::PAUSE);
		return;
	}

	// 更新
	stage_->Update();
	itemMng_->Update();
	player_->Update();
	enemyManager_->Update();
	skyDome_->Update();

}

void GameScene::Draw(void)
{
	skyDome_->Draw();
	stage_->Draw();
	itemMng_->Draw();

	// 影描画
	DrawShadow();

	enemyManager_->Draw();
	player_->Draw();
	stage_->DrawUI();
;	if (isPaused_)
	{
		DrawPauseMenu();
	}
}

void GameScene::UpdatePauseMenu(void)
{
	auto& ins = InputManager::GetInstance();

	bool up = ins.IsTriggerd("Up");
	bool down = ins.IsTriggerd("Down");

	constexpr int MENU_MAX = static_cast<int>(PAUSE_MENU::MAX);

	if (up)
	{
		pauseMenuIndex_ = (pauseMenuIndex_ <= 0) ? MENU_MAX - 1 : pauseMenuIndex_ - 1;
	}
	if (down)
	{
		pauseMenuIndex_ = (pauseMenuIndex_ < 0 || pauseMenuIndex_ >= MENU_MAX - 1) ? 0 : pauseMenuIndex_ + 1;
	}

	// ===== マウスでの選択 =====
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// マウスが動いた時だけホバー判定を行う
	bool mouseMoved = (mouseX != prevMouseX_ || mouseY != prevMouseY_);

	if (mouseMoved)
	{
		const char* labels[MENU_MAX] = { "Resume", "Option", "Back to Title" };

		int baseY = screenH / 2 - 20;
		constexpr int ITEM_SPAN = 110;
		constexpr int MENU_FONT = 70;

		int prevSize = GetFontSize();
		SetFontSize(MENU_FONT);

		int mouseHoverIndex = -1;
		for (int i = 0; i < MENU_MAX; i++)
		{
			int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
			int x = (screenW - textW) / 2;
			int y = baseY + i * ITEM_SPAN;

			if (mouseX >= x - 60 && mouseX <= x + textW &&
				mouseY >= y && mouseY <= y + ITEM_SPAN)
			{
				mouseHoverIndex = i;
				break;
			}
		}

		SetFontSize(prevSize);

		pauseMenuIndex_ = mouseHoverIndex;
	}

	prevMouseX_ = mouseX;
	prevMouseY_ = mouseY;

	// 決定（マウス左クリック追加）
	bool decide = ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsTrgDown(KEY_INPUT_SPACE)
		|| ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN)
		|| (GetMouseInput() & MOUSE_INPUT_LEFT);
	if (!decide || pauseMenuIndex_ < 0) return;

	switch (static_cast<PAUSE_MENU>(pauseMenuIndex_))
	{
	case PAUSE_MENU::RESUME:
		isPaused_ = false;
		break;
	case PAUSE_MENU::OPTION:
		break;
	case PAUSE_MENU::TITLE:
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		SoundManager::GetInstance().StopWalk();
		break;
	default: break;
	}
}

void GameScene::DrawPauseMenu(void)
{
	int screenW, screenH;
	GetScreenState(&screenW, &screenH, nullptr);

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
	DrawBox(0, 0, screenW, screenH, 0x000000, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int prevSize = GetFontSize();

	constexpr int MENU_MAX = static_cast<int>(PAUSE_MENU::MAX);
	constexpr int MENU_FONT = 70;
	SetFontSize(MENU_FONT);

	const char* labels[MENU_MAX] =
	{
		"Resume",
		"Option",
		"Back to Title",
	};

	constexpr int ITEM_SPAN = 110;
	// 3項目を画面縦中央に配置（中央の項目がscreenH/2に来るよう逆算）
	int totalHeight = ITEM_SPAN * (MENU_MAX - 1);
	int baseY = (screenH - totalHeight) / 2;

	for (int i = 0; i < MENU_MAX; i++)
	{
		bool selected = (i == pauseMenuIndex_);
		unsigned int color = selected ? 0xffff60 : 0xaaaaaa;

		int textW = GetDrawStringWidth(labels[i], (int)strlen(labels[i]));
		int x = (screenW - textW) / 2;
		int y = baseY + i * ITEM_SPAN;

		if (selected)
		{
			DrawString(x - 60, y, ">", color);
		}
		DrawString(x, y, labels[i], color);
	}

	SetFontSize(prevSize);
}


// リアルシャドウ描画
void GameScene::DrawShadow(void)
{
	// シャドウマップハンドルの作成
	int ShadowMapHandle = MakeShadowMap(1024, 1024);
	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(ShadowMapHandle, { 0.3f, -0.7f, 0.8f });
	// シャドウマップに描画する範囲を設定
	SetShadowMapDrawArea(ShadowMapHandle, 
		VGet(player_->GetTransform().pos.x - 1000.0f, player_->GetTransform().pos.y - 1.0f, player_->GetTransform().pos.z -1000.0f), 
		VGet(player_->GetTransform().pos.x + 1000.0f, player_->GetTransform().pos.y + 1000.0f, player_->GetTransform().pos.z + 1000.0f));

	// シャドウマップへの描画の準備
	ShadowMap_DrawSetup(ShadowMapHandle);

	// シャドウマップへステージモデルの描画
	MV1DrawModel(stage_->GetTransform().modelId);
	// シャドウマップへキャラクターモデルの描画
	MV1DrawModel(player_->GetTransform().modelId);

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();

	// 描画に使用するシャドウマップを設定
	SetUseShadowMap(0, ShadowMapHandle);

	// ステージモデルの描画
	MV1DrawModel(stage_->GetTransform().modelId);
	// キャラクターモデルの描画
	MV1DrawModel(player_->GetTransform().modelId);

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);

	// シャドウマップの削除
	DeleteShadowMap(ShadowMapHandle);
}
