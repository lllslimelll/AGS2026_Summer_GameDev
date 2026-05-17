#pragma once
class Player;
class HpManager
{
public:
	// ＨＰ描画の開始座標(X)
	static constexpr int START_X = 60;
	// ＨＰ描画の開始座標(Y)
	static constexpr int START_Y = 50;
	// ＨＰアイコンの描画幅
	static constexpr int HP_ICON_WIDTH = 100;

	// コンストラクタ
	HpManager(Player* player);
	// デストラクタ
	~HpManager(void);
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);
private:
	// プレイヤー情報
	Player* player_;
	// ＨＰアイコン画像ハンドル
	int imgHeart_;
	// 失ったＨＰアイコン画像ハンドル
	int imgHeartLost_;
};

