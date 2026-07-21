// Game/Actor/ActorBase.h
#pragma once
#include <vector>
#include <memory>
#include "../../Component/ActorComponent.h"
#include "../../Component/SceneComponent.h"
#include "../../Core/Vector3.h"
#include "../../Core/Quaternion.h"
struct HitResult;
class ResourceManager;
class SceneManager;

class ActorBase
{
public:

    ActorBase(void);
    virtual ~ActorBase(void);

    // ---------------------------------------------------------------
    // ライフサイクル
    // Component を全部自動で呼ぶ
    // 派生クラスは ActorBase::Init() を先に呼ぶ
    // ---------------------------------------------------------------
    virtual void Init(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Release(void);

    // ---------------------------------------------------------------
    // 当たり判定通知
    // ---------------------------------------------------------------
    virtual void OnHit(const HitResult& hit) {}
    virtual void OnOverlap(const HitResult& hit) {}

    // ---------------------------------------------------------------
    // Component 管理
    // ---------------------------------------------------------------
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        auto comp = std::make_unique<T>(
            *this,
            std::forward<Args>(args)...);
        T* ptr = comp.get();

        // SceneComponent なら RootComponent に attach
        if (auto* scene = dynamic_cast<SceneComponent*>(ptr))
        {
            scene->AttachTo(rootComponent_);
        }

        components_.push_back(std::move(comp));
        return ptr;
    }

    template<typename T>
    T* GetComponent(void) const
    {
        for (auto& comp : components_)
        {
            if (auto* ptr = dynamic_cast<T*>(comp.get()))
            {
                return ptr;
            }
        }
        return nullptr;
    }

    // ---------------------------------------------------------------
    // Transform アクセス（RootComponent 経由）
    // ---------------------------------------------------------------
    Vector3    GetPos(void) const;
    Quaternion GetRot(void) const;
    Vector3    GetScl(void)    const;

    void SetPos(const Vector3& pos);
    void SetRot(const Quaternion& rot);
    void SetScl(const Vector3& scl);

    Vector3 GetForward(void) const;
    Vector3 GetRight(void)   const;
    Vector3 GetUp(void)      const;

    SceneComponent* GetRootComponent(void) const;

protected:

    ResourceManager& resMng_;
    SceneManager& scnMng_;

    SceneComponent* rootComponent_ = nullptr;
    std::vector<std::unique_ptr<ActorComponent>> components_;
};