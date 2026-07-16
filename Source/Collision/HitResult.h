#pragma once
#include<DxLib.h>
#include "CollisionChannel.h"
#include "CollisionResponse.h"  

class ColliderBase;
class ActorBase;

struct HitResult
{
	// 衝突した自分のコライダー
	ColliderBase* selfCollider = nullptr;
	// 衝突した相手のコライダー
	ColliderBase* otherCollider = nullptr;
	// 衝突した相手のActor
	ActorBase* otherActor = nullptr;

	// 衝突点（ワールド座標）
	VECTOR point = { 0.0f,0.0f,0.0f };
	// 衝突法線
	VECTOR normal = { 0.0f,0.0f,0.0f };
	// 押し戻し後の座標
	VECTOR pushBackPos = { 0.0f,0.0f,0.0f };

	// 衝突への反応
	CollisionResponse response = CollisionResponse::NONE;

	// 衝突しているか
	bool isHit = false;
	// 衝突までの距離
	float distance = 0.0f;
	// BLOCK で当たったか
	bool isBlocking = false;
};