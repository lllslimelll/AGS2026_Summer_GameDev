#include <DxLib.h>
#include "../Application.h"
#include "../Core/Vector2.h"
#include "../Manager/ScreenManager.h"
#include "../Renderer/Material/Material.h"
#include "../Renderer/PixelRenderer.h"
#include "PostEffect.h"

PostEffect::PostEffect(void)
    : targetScreen_(-1),
    pingPongScreens_{ -1, -1 },
    enabled_(false)
{
}

PostEffect::~PostEffect(void)
{
}

void PostEffect::Init(int targetScreen)
{
    targetScreen_ = targetScreen;

    pingPongScreens_[0] = ScreenManager::GetInstance().GetPingPongScreen(0);
    pingPongScreens_[1] = ScreenManager::GetInstance().GetPingPongScreen(1);

    InitEffect();
}

void PostEffect::Draw(void)
{
    if (!enabled_) return;

    int currentRead = targetScreen_;
    int currentWrite = pingPongScreens_[0];

    for (int i = 0; i < (int)materials_.size(); i++)
    {
        SetDrawScreen(currentWrite);
        ClearDrawScreen();

        // 入力テクスチャを読み込み元に更新
        materials_[i]->SetTexture(0, currentRead);

        renderers_[i]->Draw();

        // ピンポン
        currentRead = currentWrite;
        currentWrite = (currentWrite == pingPongScreens_[0])
            ? pingPongScreens_[1]
            : pingPongScreens_[0];
    }

    // 最終結果を対象スクリーンに描画
    SetDrawScreen(targetScreen_);
    DrawGraph(0, 0, currentRead, true);
}

void PostEffect::SetEnabled(bool enabled)
{
    enabled_ = enabled;
}

void PostEffect::AddEffect(
    const std::string& psFile, int constBufSize,
    int texSlotNum, int texAddress)
{
    auto material = std::make_unique<Material>(
        psFile, constBufSize, texSlotNum, texAddress);

    auto renderer = std::make_unique<PixelRenderer>(*material);
    renderer->MakeSquareVertex(
        Vector2(Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y),
        Vector2(0, 0));

    materials_.push_back(std::move(material));
    renderers_.push_back(std::move(renderer));
}

// ---------------------------------------------------------------
// SetConstBuffer オーバーロード（指定しなかった成分は 0.0f）
// ---------------------------------------------------------------
void PostEffect::SetConstBuffer(int effectIndex, int bufIndex, float x)
{
    SetConstBuffer(effectIndex, bufIndex, x, 0.0f, 0.0f, 0.0f);
}
void PostEffect::SetConstBuffer(int effectIndex, int bufIndex, float x, float y)
{
    SetConstBuffer(effectIndex, bufIndex, x, y, 0.0f, 0.0f);
}
void PostEffect::SetConstBuffer(int effectIndex, int bufIndex, float x, float y, float z)
{
    SetConstBuffer(effectIndex, bufIndex, x, y, z, 0.0f);
}
void PostEffect::SetConstBuffer(int effectIndex, int bufIndex, float x, float y, float z, float w)
{
    materials_[effectIndex]->SetConstPS(bufIndex, x, y, z, w);
}