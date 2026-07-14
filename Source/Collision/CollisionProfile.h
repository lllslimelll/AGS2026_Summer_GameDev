#pragma once
#include <array>
#include "CollisionChannel.h"
#include "CollisionResponse.h"
#include "CollisionProfileType.h"

// コライダーの衝突プロファイル
struct CollisionProfile
{
	// プロファイルの種別
	CollisionProfileType type_;

	// 自分が所属するチャンネル
	CollisionChannel channel_;

	// コンストラクタ
	CollisionProfile(void) = default;
	CollisionProfile(CollisionProfileType type, CollisionChannel channel);

	// 相手チャンネルへの反応を設定する
	void SetResponse(CollisionChannel channel, CollisionResponse response);

	// 相手チャンネルへの反応を取得する
	CollisionResponse GetResponse(CollisionChannel channel) const;

private:

	// チャンネルの数
	static constexpr int CHANNEL_COUNT = static_cast<int>(CollisionChannel::MAX);

	// 相手チャンネルごとの反応
	std::array<CollisionResponse, CHANNEL_COUNT> responses_;
};
