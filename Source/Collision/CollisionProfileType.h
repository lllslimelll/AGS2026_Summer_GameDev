#pragma once

enum class CollisionProfileType
{
	PAWN,		   // キャラクター全般
	WORLD_STATIC,  // 静的オブジェクト
	WORLD_DYNAMIC, // 動的オブジェクト
	PHYSICS_BODY,  // 物理演算で動くもの
	ITEM,		   // アイテム
	CAMERA,		   // カメラ
	TRIGGER,	   // トリガー判定
	VISIBILITY,	   // レイ判定

	MAX,		   // プロファイル数
};