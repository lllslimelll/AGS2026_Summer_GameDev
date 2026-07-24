// SettingsManager.h
#pragma once

class SettingsManager
{
public:
    static void CreateInstance(void);
    static SettingsManager& GetInstance(void);

    // 音量：5段階（0?4）、初期値2（3番目）
    static constexpr int VOLUME_STEPS = 5;
    static constexpr int VOLUME_DEFAULT = 2;

    // 感度：9段階（0?8）、初期値4（5番目）
    static constexpr int SENS_STEPS = 9;
    static constexpr int SENS_DEFAULT = 4;

    int bgmVolumeStep = VOLUME_STEPS - 1;
    int seVolumeStep = VOLUME_STEPS -1;
    int mouseSensStep = SENS_DEFAULT;
    int padSensStep = SENS_DEFAULT;

    // 各マネージャに反映
    void Apply(void);

    // ステップ -> 0.0~1.0
    static float StepToRate(int step, int maxSteps);

private:
    SettingsManager(void) = default;
    static SettingsManager* instance_;
};