#pragma once
#include <DxLib.h>

#define BGM_TITLE_PATH "Data/Sound/BGM_Title.mp3"
#define WALK_PATH "Data/Sound/Walk.mp3"
#define BOOST_PATH "Data/Sound/Boost.mp3"
#define PICKUP_PATH "Data/Sound/PickUp.wav"
#define DAMAGED_PATH "Data/Sound/Damaged.wav"


class SoundManager
{
public:

	// 音量（０～２５５）
	static constexpr int BGM_TITLE_VOLUME = 200; // タイトルBGM
	static constexpr int WALK_VOLUME = 200;		 // 歩き
	static constexpr int PICKUP_VOLUME = 255;	 // 取得
	static constexpr int BOOST_VOLUME = 200;	 // ブースト
	static constexpr int DAMAGED_VOLUME = 200;	 // ブースト


	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SoundManager& GetInstance(void);

	void Init(void);
	void Update(void);
	void Destroy(void);

	// 音源再生
	void PlayBgmTitle();		// タイトルBGM
	void PlayWalk();	// 歩き
	void PlayPickUp();	// 取得
	void PlayBoost();	// ブースト
	void PlayDamaged();

	//音源停止	
	void StopBGMTitle();		// タイトルBGM
	void StopWalk();	// 歩き
	void StopPickUp();	// 取得
	void StopBoost();	// ブースト
	void StopDamaged();	// ブースト


private:

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SoundManager(void);

	// コピーコンストラクタも同様
	SoundManager(const SoundManager& instance) = default;

	// デストラクタも同様
	~SoundManager(void) = default;

	// 静的インスタンス
	static SoundManager* instance_;

	int bgmTitleH_;
	int walkH_;
	int pickUpH_;
	int boostH_;
	int damagedH_;
};