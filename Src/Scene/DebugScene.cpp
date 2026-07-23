#include <fstream>
#include <DxLib.h>
#include "../Common/Vector2.h"
#include "../Manager/InputManager.h"
#include "SceneManager.h"
#include "../Camera/Camera.h"
#include "../Object/Actor/Stage/StageManager.h"
#include "../Object/Actor/Stage/Planet.h"
#include "../Object/Collider/ColliderModel.h"
#include "DebugScene.h"

DebugScene::DebugScene(void)
	:
	SceneBase(),
	stageMng_(nullptr)
{
}

DebugScene::~DebugScene(void)
{
	// ステージ解放
	stageMng_->Release();
	delete stageMng_;

	// デバッグポイント群
	points_.clear();
}

void DebugScene::Init(void)
{
	// ステージ生成
	stageMng_ = new StageManager();
	stageMng_->Init();

	// カメラ
	SceneManager::GetInstance().GetCamera().ChangeMode(Camera::MODE::FREE);
}

void DebugScene::Update(void)
{
	// ステージ更新
	stageMng_->Update();
}

void DebugScene::Draw(void)
{
	// ステージ描画
	stageMng_->Draw();

	// デバッグポイントの配置
	PlaceDebugPoint();

	// デバッグポイント群を球体描画
	int y = 20;
	for (const auto& point : points_)
	{
		DrawSphere3D(
			point,
			30.0f,
			16,
			GetColor(255, 0, 0),
			GetColor(255, 0, 0),
			false );

		DrawFormatString(20, y,
			0x000000, "座標（%.2f, %.2f, %.2f）",
			point.x, point.y, point.z);

		y += 20;
	}
}

void DebugScene::PlaceDebugPoint(void)
{
	const auto& ins = InputManager::GetInstance();

	// クリックした場所にデバッグポイント群を設置
	if (ins.IsTriggered(InputManager::InputCommand::SET_POINT))
	{
		// マウス座標の取得
		Vector2 mousePos = ins.GetMousePos();

		// スクリーン座標をVECTOR構造体に変換
		VECTOR screenPos = VECTOR();
		screenPos.x = static_cast<float>(mousePos.x);
		screenPos.y = static_cast<float> (mousePos.y);
		// ｚが１．０ｆでカメラの最奥になる
		screenPos.z = 1.0f;

		// マウスの２Ｄ座標から３Ｄ座標へ変換
		VECTOR worldPos = ConvScreenPosToWorldPos(screenPos);

		// ステージのモデルコライダを取得
		const ColliderBase* collder = stageMng_->GetPlanet().GetOwnCollider(
			static_cast<int>(Planet::COLLIDER_TYPE::MODEL));
		
		if (collder == nullptr) return;

		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(collder);

		// カメラ情報を取得
		const auto& camera = SceneManager::GetInstance().GetCamera();
		
		// カメラの位置からカメラ最奥のワールド座標へ向けてレイを飛ばす
		auto hit = MV1CollCheck_Line(
			colliderModel->GetFollow()->modelId, -1,
			camera.GetPos(),
			worldPos);

		if (hit.HitFlag)
		{
			//衝突地点をデバッグポイント群に追加
			points_.push_back(hit.HitPosition);
		}
		printfDx("worldPos: %.1f, %.1f, %.1f\n", worldPos.x, worldPos.y, worldPos.z);
		printfDx("camPos:   %.1f, %.1f, %.1f\n", camera.GetPos().x, camera.GetPos().y, camera.GetPos().z);
	}

	// 右クリックで最後のデバッグポイントを削除
	if (ins.IsTriggered(InputManager::InputCommand::DELETE_POINT))
	{
		if (points_.size() > 0)
		{
			points_.pop_back();
		}
	}

	// デバッグポイントの保存
	if (ins.IsTriggered(InputManager::InputCommand::SAVE_POINT))
	{
		SavePoints();
	}

}

void DebugScene::SavePoints(void)
{
	std::ofstream ofs("Data/Csv/PointSave.txt");
	if (!ofs) return;

	// 形式: x y z
	for (const VECTOR& p : points_) {
		ofs << p.x << " " << p.y << " " << p.z << "\n";
	}

	ofs.close();
}
