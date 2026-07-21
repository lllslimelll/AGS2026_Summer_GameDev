#include "../../../Manager/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Component/StaticMeshComponent.h"
#include "Planet.h"

Planet::~Planet(void) {}

void Planet::Init(void)
{
	int modelId = resMng_.Load(ResourceManager::SRC::MAIN_STAGE).handleId_;
	mesh_ = AddComponent<StaticMeshComponent>(modelId);
	mesh_->SetProfile(CollisionProfileType::WORLD_STATIC);

	for (const std::string& name : EXCLUDE_FRAME_NAMES)
		mesh_->AddExcludeFrame(name);
	for (const std::string& name : TARGET_FRAME_NAMES)
		mesh_->AddIncludeFrame(name);

	SetScl(Vector3(2.0f, 2.0f, 2.0f));
	SetPos(Vector3::ZERO);

	ActorBase::Init();
}

void Planet::Update(void) { ActorBase::Update(); }
void Planet::Draw(void) { ActorBase::Draw(); }