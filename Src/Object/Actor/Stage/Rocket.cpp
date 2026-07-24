#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SoundManager.h"
#include "../../../Utility/AsoUtility.h"
#include "../../Collider/ColliderModel.h"
#include "Rocket.h"

Rocket::~Rocket(void)
{
}

void Rocket::Update(void)
{
    transform_.Update();

    UpdateCurvatureShader();
}

void Rocket::Draw(void)
{
    ActorBase::Draw();
}

void Rocket::AddDelivery(int value)
{
    totalDelivered_ += value;
    SoundManager::GetInstance().PlayPickUp();
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
    transform_.scl = { 4,4,4 };
    transform_.pos = { -1100, -100, -8700.0f };

    transform_.quaRotLocal = Quaternion::AngleAxis(AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y);

    transform_.Update();
}

void Rocket::InitCollider(void)
{
    MV1SetupCollInfo(transform_.modelId);

    ColliderModel* colModel =
        new ColliderModel(ColliderBase::TAG::ROCKET, &transform_);

    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Rocket::InitAnimation(void)
{
}

void Rocket::InitPost(void)
{
    transform_.Update();

    InitCurvatureItemShader();
}