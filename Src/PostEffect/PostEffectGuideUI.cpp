#include "../Application.h"
#include "PostEffectGuideUI.h"

PostEffectGuideUI::PostEffectGuideUI(void)
{
}

PostEffectGuideUI::~PostEffectGuideUI(void)
{
}

void PostEffectGuideUI::InitEffect(void)
{
    Add((Application::PATH_SHADER + "UIPS.pso").c_str(),
        CONST_BUF_FLOAT4_SIZE, TEX_SLOT_NUM, DX_TEXADDRESS_CLAMP);

    distortion_ = 0.25f; // óví≤êÆ
    SetConstBuffer(0, 0, { distortion_, 0.0f, 0.0f, 0.0f });
}

void PostEffectGuideUI::Update()
{
	if (CheckHitKey(KEY_INPUT_P)) distortion_ += 0.1;
	if (CheckHitKey(KEY_INPUT_L)) distortion_ -= 0.1;
	SetConstBuffer(0, 0, { distortion_, 0.0f, 0.0f, 0.0f });
}
