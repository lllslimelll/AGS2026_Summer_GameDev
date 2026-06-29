#pragma once
#include "Scene"

class PauseScene : public Scene
{
	PauseScene();
	~PauseScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;
};