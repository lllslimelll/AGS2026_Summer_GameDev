#include "../../../Manager/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Collision/ColliderModel.h"
#include "Rocket.h"

Rocket::~Rocket(void)
{
}

void Rocket::Update(void)
{
    transform_.Update();
}

void Rocket::Draw(void)
{
    ActorBase::Draw();
}

void Rocket::AddDelivery(int value)
{
    totalDelivered_ += value;
}

int Rocket::GetTotalDelivered(void) const
{
    return totalDelivered_;
}

bool Rocket::IsQuotaCleared(void) const
{
    return totalDelivered_ >= QUOTA;
}

const VECTOR& Rocket::GetPos(void) const
{
    return transform_.pos;
}

void Rocket::InitLoad(void)
{
    transform_.SetModel(resMng_.Load(
        ResourceManager::SRC::ROKET).handleId_);
}

void Rocket::InitTransform(void)
{
    transform_.pos = { 1300, -860, 430 };

    Quaternion rotX = Quaternion::AngleAxis(AsoUtility::Deg2RadF(15.0f), AsoUtility::AXIS_X);
    Quaternion rotY = Quaternion::AngleAxis(AsoUtility::Deg2RadF(0.0f), AsoUtility::AXIS_Y);
    Quaternion rotZ = Quaternion::AngleAxis(AsoUtility::Deg2RadF(-130.0f), AsoUtility::AXIS_Z);

    transform_.quaRotLocal = rotZ.Mult(rotY).Mult(rotX);

    transform_.Update();
}

void Rocket::InitCollider(void)
{
    MV1SetupCollInfo(transform_.modelId);

    ColliderModel* colModel =
        new ColliderModel(CollisionProfileType::WORLD_DYNAMIC, this);

    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Rocket::InitAnimation(void)
{
}

void Rocket::InitPost(void)
{
    transform_.Update();
}