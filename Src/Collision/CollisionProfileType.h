#pragma once

enum class CollisionProfileType
{
	NONE,		// 未設定
	PAWN,		// キャラクター全般
	WORLD_STATIC,  // 静的オブジェクト
	WORLD_DYNAMIC, // 動的オブジェクト
	PHYSICS_BODY,  // 物理演算で動くもの
	CAMERA,		   // カメラ
	TRIGGER,	   // トリガー判定
	VISIBILITY	   // レイ判定
};