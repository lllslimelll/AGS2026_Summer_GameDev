#include <DxLib.h>
#include "../../Collider/ColliderBase.h"
#include "Planet.h"
#include "Rocket.h"
#include "StageManager.h"

StageManager::StageManager(void)
{
}

StageManager::~StageManager(void)
{
}

void StageManager::Init(void)
{
    planet_ = std::make_unique<Planet>();
    planet_->Init();

    rocket_ = std::make_unique<Rocket>();
    rocket_->Init();
}

void StageManager::Update(void)
{
    planet_->Update();
    rocket_->Update();
}

void StageManager::Draw(void)
{
    planet_->Draw();
    rocket_->Draw();
    DrawUI();
}

void StageManager::DrawUI(void)
{
    int prevSize = GetFontSize();
    SetFontSize(55);

    auto withComma = [](int value, char* out)
        {
            char tmp[32];
            sprintf_s(tmp, 32, "%d", value);
            int len = (int)strlen(tmp);
            int outIdx = 0;
            int firstLen = len % 3;
            if (firstLen == 0) firstLen = 3;
            for (int i = 0; i < firstLen; i++) out[outIdx++] = tmp[i];
            for (int i = firstLen; i < len; i += 3)
            {
                out[outIdx++] = ',';
                out[outIdx++] = tmp[i];
                out[outIdx++] = tmp[i + 1];
                out[outIdx++] = tmp[i + 2];
            }
            out[outIdx] = '\0';
        };

    char totalStr[32];
    char quotaStr[32];
    withComma(rocket_->GetTotalDelivered(), totalStr);
    withComma(5000, quotaStr);

    char buf[64];
    sprintf_s(buf, "$%s / $%s", totalStr, quotaStr);
    int textW = GetDrawStringWidth(buf, (int)strlen(buf));

    constexpr int SCREEN_W = 1920;
    constexpr int MARGIN = 50;
    int x = SCREEN_W - textW - MARGIN;
    int y = MARGIN;

    unsigned int color = rocket_->IsQuotaCleared() ? 0x00ff00 : 0xffffff;
    DrawFormatString(x, y, color, "%s", buf);

    SetFontSize(prevSize);
}

void StageManager::Release(void)
{
    planet_->Release();
    rocket_->Release();
}

Rocket& StageManager::GetRocket(void)
{
    return *rocket_;
}

Planet& StageManager::GetPlanet(void)
{
    return *planet_;
}

const Transform& StageManager::GetTransform(void) const
{
    return planet_->GetTransform();
}