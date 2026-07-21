#include "../../../Utility/Math.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Component/StaticMeshComponent.h"
#include "Rocket.h"

Rocket::~Rocket(void) {}

void Rocket::Init(void)
{
	int modelId = resMng_.Load(ResourceManager::SRC::ROKET).handleId_;
	mesh_ = AddComponent<StaticMeshComponent>(modelId);
	mesh_->SetProfile(CollisionProfileType::WORLD_STATIC);

	using Q = Quaternion;
	Q rotX = Q::AngleAxis(Math::ToRadian(15.0f), Vector3::RIGHT);
	Q rotY = Q::AngleAxis(Math::ToRadian(0.0f), Vector3::UP);
	Q rotZ = Q::AngleAxis(Math::ToRadian(-130.0f), Vector3::FORWARD);
	SetRot(rotZ * rotY * rotX);
	SetPos(Vector3(1300.0f, -860.0f, 430.0f));

	ActorBase::Init();
}

void Rocket::Update(void) { ActorBase::Update(); }
void Rocket::Draw(void) { ActorBase::Draw(); }

void Rocket::AddDelivery(int value) { totalDelivered_ += value; }
int  Rocket::GetTotalDelivered(void) const { return totalDelivered_; }
bool Rocket::IsQuotaCleared(void) const { return totalDelivered_ >= QUOTA; }
int  Rocket::GetModelId(void) const { return mesh_ ? mesh_->GetModelId() : -1; }