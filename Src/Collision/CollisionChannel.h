#pragma once

enum class CollisionChannel
{
	NONE,		   // 未設定
	WORLD_STATIC,  // 静的オブジェクト用
	WORLD_DYNAMIC, // 動的オブジェクト用
	PHYSICS_BODY,  // 物理演算で動く用
	PAWN,		   // キャラクター全般用
	VISIBILITY,    // レイキャスト用
	CAMERA,		   // カメラ用
	TRIGGER,	    // トリガー判定
};