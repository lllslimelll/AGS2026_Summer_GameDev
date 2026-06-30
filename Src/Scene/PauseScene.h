#pragma once
#include "Scene.h"

class PauseScene : public Scene
{
public:

	PauseScene();
	~PauseScene() override;

	void Init() override;
	void Update() override;
	void Draw() override;
};