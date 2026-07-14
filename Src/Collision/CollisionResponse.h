#pragma once

// 衝突反応
// 値の大きいほうが優先される（BLOCK > OVERLAP > IGNORE）
enum class CollisionResponse
{
	IGNORE  = 0, // 判定しない

	OVERLAP = 1, // 検知のみ

	BLOCK   = 2	 // 物理応答あり
};