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
    return profiles_[static_cast<int>(type)];
}

void CollisionProfileManager::RegisterPresets(void)
{
    // 当たり判定に参加しない（デフォルト）
    CollisionProfile noCollision(
        CollisionProfileType::NO_COLLISION,
        CollisionChannel::NONE);
    // 全レスポンス NONE のまま
    profiles_[static_cast<int>(CollisionProfileType::NO_COLLISION)] = noCollision;

    // キャラ-----------------------------------------------------------------------
    CollisionProfile pawn(
        CollisionProfileType::PAWN,
        CollisionChannel::PAWN);

    pawn.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);
    pawn.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);
    pawn.SetResponse(CollisionChannel::PAWN, CollisionResponse::BLOCK);
    pawn.SetResponse(CollisionChannel::PHYSICS_BODY, CollisionResponse::OVERLAP);
    pawn.SetResponse(CollisionChannel::TRIGGER, CollisionResponse::OVERLAP);
    pawn.SetResponse(CollisionChannel::VISIBILITY, CollisionResponse::BLOCK);

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

    // 静的オブジェクト--------------------------------------------------------------
    // ※未登録だとデフォルト（channel NONE / 全レスポンス NONE）になり
    //   キャラとステージの判定自体がスキップされてすり抜ける
    CollisionProfile worldStatic(
        CollisionProfileType::WORLD_STATIC,
        CollisionChannel::WORLD_STATIC);

    worldStatic.SetResponse(CollisionChannel::PAWN, CollisionResponse::BLOCK);
    worldStatic.SetResponse(CollisionChannel::ITEM, CollisionResponse::BLOCK);
    worldStatic.SetResponse(CollisionChannel::CAMERA, CollisionResponse::BLOCK);
    worldStatic.SetResponse(CollisionChannel::VISIBILITY, CollisionResponse::BLOCK);
    worldStatic.SetResponse(CollisionChannel::PHYSICS_BODY, CollisionResponse::BLOCK);
    worldStatic.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);

    profiles_[static_cast<int>(CollisionProfileType::WORLD_STATIC)] = worldStatic;

    // 動的オブジェクト--------------------------------------------------------------
    CollisionProfile worldDynamic(
        CollisionProfileType::WORLD_DYNAMIC,
        CollisionChannel::WORLD_DYNAMIC);

    worldDynamic.SetResponse(CollisionChannel::PAWN, CollisionResponse::BLOCK);
    worldDynamic.SetResponse(CollisionChannel::ITEM, CollisionResponse::BLOCK);
    worldDynamic.SetResponse(CollisionChannel::CAMERA, CollisionResponse::BLOCK);
    worldDynamic.SetResponse(CollisionChannel::VISIBILITY, CollisionResponse::BLOCK);
    worldDynamic.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);

    profiles_[static_cast<int>(CollisionProfileType::WORLD_DYNAMIC)] = worldDynamic;

    // 物理演算体--------------------------------------------------------------------
    CollisionProfile physicsBody(
        CollisionProfileType::PHYSICS_BODY,
        CollisionChannel::PHYSICS_BODY);

    physicsBody.SetResponse(CollisionChannel::WORLD_STATIC, CollisionResponse::BLOCK);
    physicsBody.SetResponse(CollisionChannel::WORLD_DYNAMIC, CollisionResponse::BLOCK);
    physicsBody.SetResponse(CollisionChannel::PAWN, CollisionResponse::OVERLAP);

    profiles_[static_cast<int>(CollisionProfileType::PHYSICS_BODY)] = physicsBody;
}




