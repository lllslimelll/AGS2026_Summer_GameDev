#include "CollisionProfileManager.h"

CollisionProfileManager* CollisionProfileManager::instance_ = nullptr;

void CollisionProfileManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new CollisionProfileManager();
	}
}

CollisionProfileManager& CollisionProfileManager::GetInstance(void)
{
    if (instance_ == nullptr)
    {
        CreateInstance();
    }

    return *instance_;
}

void CollisionProfileManager::Destroy(void)
{
    delete instance_;
    instance_ = nullptr;
}

CollisionProfileManager::CollisionProfileManager(void)
{
    RegisterPresets();
}

const CollisionProfile& CollisionProfileManager::GetProfile(CollisionProfileType type) const
{
    int index = static_cast<int>(type);

    return profiles_[index];
}

void CollisionProfileManager::RegisterPresets(void)
{
    // キャラ-----------------------------------------------------------------------
    CollisionProfile pawn(
        CollisionProfileType::PAWN,
        CollisionChannel::PAWN);

    pawn.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);
    pawn.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);
    pawn.SetResponse(CollisionChannel::PAWN, CollisionResponse::BLOCK);
    pawn.SetResponse(CollisionChannel::PHYSICS_BODY, CollisionResponse::OVERLAP);
    pawn.SetResponse(CollisionChannel::TRIGGER, CollisionResponse::OVERLAP);

    // カメラ------------------------------------------------------------------------
    profiles_[static_cast<int>(CollisionProfileType::PAWN)] = pawn;

    CollisionProfile camera(
        CollisionProfileType::CAMERA,
        CollisionChannel::CAMERA);

    camera.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);
    camera.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);

    profiles_[static_cast<int>(CollisionProfileType::CAMERA)] = camera;

    // アイテム----------------------------------------------------------------------
    CollisionProfile item(
        CollisionProfileType::ITEM,
        CollisionChannel::ITEM);

    item.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);
    item.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);

    profiles_[static_cast<int>(CollisionProfileType::ITEM)] = item;

    // トリガー判定-------------------------------------------------------------------
    CollisionProfile trigger(
        CollisionProfileType::TRIGGER,
        CollisionChannel::TRIGGER);

    trigger.SetResponse(CollisionChannel::PAWN, CollisionResponse::OVERLAP);

    profiles_[static_cast<int>(CollisionProfileType::TRIGGER)] = trigger;

    // レイキャスト--------------------------------------------------------------------
    CollisionProfile visibility(
        CollisionProfileType::VISIBILITY,
        CollisionChannel::VISIBILITY);

    visibility.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);
    visibility.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);
    visibility.SetResponse(CollisionChannel::PAWN, CollisionResponse::BLOCK);
    visibility.SetResponse(CollisionChannel::ITEM, CollisionResponse::BLOCK);

    profiles_[static_cast<int>(CollisionProfileType::VISIBILITY)] = visibility;
}




