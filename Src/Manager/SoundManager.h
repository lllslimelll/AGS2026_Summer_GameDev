#pragma once
#include <DxLib.h>

#define BGM_TITLE_PATH "Data/Sound/BGM_Title.mp3"
#define BGM_GAME_PATH "Data/Sound/BGM_Game.mp3"
#define WALK_PATH "Data/Sound/Walk.mp3"
#define BOOST_PATH "Data/Sound/Boost.wav"
#define PICKUP_PATH "Data/Sound/PickUp.wav"
#define DAMAGED_PATH "Data/Sound/Damaged.wav"
#define SELECT_PATH "Data/Sound/Select.wav"
#define RESULT_PATH "Data/Sound/Result.wav"
#define ALERT_PATH  "Data/Sound/Alert.mp3"
#define DAMAGED2_PATH "Data/Sound/Damaged2.wav"


class SoundManager
{
public:

	// 音量（０～２５５）
	static constexpr int BGM_TITLE_VOLUME = 255; // タイトルBGM
	static constexpr int BGM_GAME_VOLUME = 255; //  ゲームBGM
	static constexpr int WALK_VOLUME = 255;		 // 歩き
	static constexpr int PICKUP_VOLUME = 255;	 // 取得
	static constexpr int BOOST_VOLUME = 255;	 // ブースト
	static constexpr int DAMAGED_VOLUME = 255;	 // ブースト
	static constexpr int SELECT_VOLUME = 255;  // 選択SE
	static constexpr int RESULT_VOLUME = 255;  // リザルトSE
	static constexpr int ALERT_VOLUME = 255;
	static constexpr int DAMAGED2_VOLUME = 255;


	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SoundManager& GetInstance(void);

	void Init(void);
	void Update(void);
	void Destroy(void);

	// 音源再生
	void PlayBgmTitle();		// タイトルBGM
	void PlayBGMGame();
	void PlayWalk();	// 歩き
	void PlayPickUp();	// 取得
	void PlayBoost();	// ブースト
	void PlayDamaged();
	void PlaySelect();
	void PlayResult();
	void PlayAlert();
	void PlayDamaged2();



	//音源停止	
	void StopBGMTitle();		// タイトルBGM
	void StopBGMGame();
	void StopWalk();	// 歩き
	void StopAlert();
	void StopPickUp();	// 取得
	void StopBoost();	// ブースト
	void StopDamaged();	// ブースト


	void SetBgmVolume(int vol);  // 0?255
	void SetSeVolume(int vol);   // 0?255

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
	int bgmGameH_;
	int walkH_;
	int pickUpH_;
	int boostH_;
	int damagedH_;
	int selectH_;
	int resultH_;
	int alertH_;
	int damaged2H_;
};