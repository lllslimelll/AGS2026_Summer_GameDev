// SettingsManager.cpp
#include "SettingsManager.h"
#include "SoundManager.h"

SettingsManager* SettingsManager::instance_ = nullptr;

void SettingsManager::CreateInstance(void)
{
    if (instance_ == nullptr) instance_ = new SettingsManager;
}

SettingsManager& SettingsManager::GetInstance(void)
{
    return *instance_;
}

float SettingsManager::StepToRate(int step, int maxSteps)
{
    return static_cast<float>(step) / static_cast<float>(maxSteps - 1);
}

void SettingsManager::Apply(void)
{
    auto& snd = SoundManager::GetInstance();

    // ‹Ï“™5’iŠKF0, 63, 127, 191, 255
    int bgmVol = static_cast<int>(StepToRate(bgmVolumeStep, VOLUME_STEPS) * 255.0f);
    int seVol = static_cast<int>(StepToRate(seVolumeStep, VOLUME_STEPS) * 255.0f);

    snd.SetBgmVolume(bgmVol);
    snd.SetSeVolume(seVol);
}