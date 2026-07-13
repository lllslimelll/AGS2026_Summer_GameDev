#pragma once

// ゲーム内のスクリーン(レンダーターゲット)を一元管理するクラス
class ScreenManager
{
public:

    static void CreateInstance(void);
    static ScreenManager& GetInstance(void);

    void Init(void);
    void Destroy(void);

    // メインスクリーンを取得
    int GetMainScreen(void) const;

    // ピンポンバッファ用スクリーンを取得
    int GetPingPongScreen(int index) const;

private:

    ScreenManager(void);
    ScreenManager(const ScreenManager&) = default;
    ~ScreenManager(void) = default;

    static ScreenManager* instance_;

    int mainScreen_;
    int pingPongScreen_[2];
};