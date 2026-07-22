#pragma once

// ゲーム内のスクリーン(レンダーターゲット)を管理するクラス
class ScreenManager
{
public:

	// インスタンスを明示的に生成
	static void CreateInstance(void);

	// インスタンスの取得
	static ScreenManager& GetInstance(void);

	void Init(void);

	void Destroy();

	// メインスクリーンを取得
	int GetMainScreen() const;

	// UI用スクリーンを取得(仮)
	int GetGuideUIScreen() const;

	// ピンポンバッファ用スクリーンを取得(仮)
	int GetPingPongScreen(int index) const;

private:

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	ScreenManager(void);
	ScreenManager(const ScreenManager& manager) = default;
	~ScreenManager(void) = default;

	// シングルトン用インスタンス
	static ScreenManager* instance_;

	// スクリーン
	int mainScreen_; // メイン
	int guideUIScreen_;   // ガイドUI
	int pingPongScreen_[2]; // ピンポンバッファ用

};