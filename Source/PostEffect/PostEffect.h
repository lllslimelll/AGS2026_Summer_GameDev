#pragma once
#include <string>
#include <vector>
#include <memory>
#include <DxLib.h>

class Material;
class PixelRenderer;

// ポストエフェクトの基底クラス
class PostEffect
{
public:

    PostEffect(void);
    virtual ~PostEffect(void);

    // 初期化。targetScreen に ScreenManager::GetMainScreen() を渡す。
    virtual void Init(int targetScreen);

    // 毎フレーム更新（定数バッファの更新が必要な派生クラスでオーバーライド）
    virtual void Update(void) {}

    // エフェクトを描画する
    void Draw(void);

    // エフェクトの有効 / 無効を設定する
    void SetEnabled(bool enabled);

protected:

    // 派生クラスでエフェクトを登録する（純粋仮想）
    virtual void InitEffect(void) = 0;

    // エフェクトを 1 つ追加する
    void AddEffect(const std::string& psFile, int constBufSize, int texSlotNum,
        int texAddress = DX_TEXADDRESS_CLAMP);

    // 定数バッファの値をセットする
    void SetConstBuffer(int effectIndex, int bufIndex, float x);
    void SetConstBuffer(int effectIndex, int bufIndex, float x, float y);
    void SetConstBuffer(int effectIndex, int bufIndex, float x, float y, float z);
    void SetConstBuffer(int effectIndex, int bufIndex, float x, float y, float z, float w);

    std::vector<std::unique_ptr<Material>>      materials_;
    std::vector<std::unique_ptr<PixelRenderer>> renderers_;

private:

    int  targetScreen_;
    int  pingPongScreens_[2];
    bool enabled_;
};