#include <DxLib.h>
#include "../Manager/InputManager.h"
#include "SceneManager.h"
#include "PauseScene.h"

PauseScene::PauseScene()
	:
	Scene()
{
}

PauseScene::~PauseScene()
{
}

void PauseScene::Init()
{
}

void PauseScene::Update()
{
	auto& ins = InputManager::GetInstance();
	if (ins.IsTriggerd("pause"))
	{
		// ƒV[ƒ“‚Ìíœ
		sceMng_.PopScene();
	}
}

void PauseScene::Draw()
{
}
