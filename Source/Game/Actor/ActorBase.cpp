// Game/Actor/ActorBase.cpp
#include "../../Manager/ResourceManager.h"
#include "../../Game/Scene/SceneManager.h"
#include "../../Component/ActorComponent.h"
#include "../../Component/SceneComponent.h"
#include "../../Collision/HitResult.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
    : resMng_(ResourceManager::GetInstance())
    , scnMng_(SceneManager::GetInstance())
{
    // RootComponent を生成
    auto root = std::make_unique<SceneComponent>(*this);
    rootComponent_ = root.get();
    components_.push_back(std::move(root));
}

ActorBase::~ActorBase(void)
{
}

// ---------------------------------------------------------------
// ライフサイクル
// ---------------------------------------------------------------

void ActorBase::Init(void)
{
    for (auto& comp : components_)
    {
        comp->Init();
    }
}

void ActorBase::Update(void)
{
    for (auto& comp : components_)
    {
        if (comp->IsActive())
        {
            comp->Update();
        }
    }
}

void ActorBase::Draw(void)
{
    for (auto& comp : components_)
    {
        if (comp->IsActive())
        {
            comp->Draw();
        }
    }
}

void ActorBase::Release(void)
{
    for (auto& comp : components_)
    {
        comp->Release();
    }
    components_.clear();
    rootComponent_ = nullptr;
}

// ---------------------------------------------------------------
// Transform アクセス
// ---------------------------------------------------------------

Vector3 ActorBase::GetPos(void) const
{
    return rootComponent_->GetWorldPos();
}

Quaternion ActorBase::GetRot(void) const
{
    return rootComponent_->GetWorldRot();
}

Vector3 ActorBase::GetScl(void) const
{
    return rootComponent_->GetWorldScl();
}

void ActorBase::SetPos(const Vector3& pos)
{
    rootComponent_->SetWorldPos(pos);
}

void ActorBase::SetRot(const Quaternion& rot)
{
    rootComponent_->SetWorldRot(rot);
}

void ActorBase::SetScl(const Vector3& scl)
{
    rootComponent_->SetLocalScl(scl);
}

Vector3 ActorBase::GetForward(void) const
{
    return rootComponent_->GetForward();
}

Vector3 ActorBase::GetRight(void) const
{
    return rootComponent_->GetRight();
}

Vector3 ActorBase::GetUp(void) const
{
    return rootComponent_->GetUp();
}

SceneComponent* ActorBase::GetRootComponent(void) const
{
    return rootComponent_;
}