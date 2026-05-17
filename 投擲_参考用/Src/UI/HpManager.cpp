#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Object/Player.h"
#include "HpManager.h"

HpManager::HpManager(Player* player)
{
	player_ = player;
}
HpManager::~HpManager(void)
{
}

void HpManager::Init(void)
{
	std::string path;
	// ２Ｄ画像のロード
	path = Application::PATH_IMAGE + "Heart.png";
	imgHeart_ = LoadGraph(path.c_str());
	path = Application::PATH_IMAGE + "HeartLost.png";
	imgHeartLost_ = LoadGraph(path.c_str());
}
void HpManager::Update(void)
{
}
void HpManager::Draw(void)
{
	// プレイヤーのＨＰ情報
	int max = Player::MAX_HP;
	int playerHp = player_->GetHp();

	for (int i = 0; i < max; i++)
	{
		int img = -1;

		// 1, 2, 3, 4, 5
		if ((i + 1) > playerHp)
		{
			// 失ったアイコン
			img = imgHeartLost_;
		}
		else
		{
			// 通常のアイコン
			img = imgHeart_;
		}
		// プレイヤーのＨＰ残量に応じて、ハート画像を切り替える
		DrawRotaGraph(START_X + HP_ICON_WIDTH * i, START_Y, 1.0, 0.0, img, true);
	}
}
void HpManager::Release(void)
{
	// ２Ｄ画像のメモリ解放
	DeleteGraph(imgHeart_);
	DeleteGraph(imgHeartLost_);
}
