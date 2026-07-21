#pragma once
#include <array>
#include "CollisionProfile.h"
#include "CollisionProfileType.h"

// コリジョンプロファイルを一元管理するクラス
class CollisionProfileManager
{
public:

	// インスタンスを明示的に生成
	static void CreateInstance(void);
	// インスタンスの取得
	static CollisionProfileManager& GetInstance(void);

	void Destroy(void);

	// プロファイルを取得する
	const CollisionProfile& GetProfile(CollisionProfileType type) const;

private:

	// シングルトン用静的インスタンス
	static CollisionProfileManager* instance_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	CollisionProfileManager(void);
	// コピーコンストラクタも同様
	CollisionProfileManager(const CollisionProfileManager& instance) = default;
	// デストラクタも同様
	~CollisionProfileManager(void) = default;

	// プロファイルテーブル
	std::array<CollisionProfile, static_cast<int>(CollisionProfileType::MAX)> profiles_;

	// プリセットプロファイルを登録する
	void RegisterPresets(void);
};

