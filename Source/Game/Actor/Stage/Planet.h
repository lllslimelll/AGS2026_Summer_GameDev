#pragma once
#include <vector>
#include <string>
#include "../ActorBase.h"

class StaticMeshComponent;

class Planet : public ActorBase
{
public:

	~Planet(void) override;

	void Init(void)   override;
	void Update(void) override;
	void Draw(void)   override;

private:

	StaticMeshComponent* mesh_ = nullptr;

	const std::vector<std::string> EXCLUDE_FRAME_NAMES = {};
	const std::vector<std::string> TARGET_FRAME_NAMES = { "Ground" };
};