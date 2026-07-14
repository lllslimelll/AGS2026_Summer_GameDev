#pragma once

enum class CollisionChannel
{
	WORLD_STATIC,  // 静的オブジェクト用
	WORLD_DYNAMIC, // 動的オブジェクト用
	PHYSICS_BODY,  // 物理演算で動く用
	PAWN,		   // キャラクター全般用
	ITEM,		   // アイテム
	VISIBILITY,    // レイキャスト用
	CAMERA,		   // カメラ用 
	TRIGGER,	   // トリガー判定

	MAX,		   // プロファイル数
};