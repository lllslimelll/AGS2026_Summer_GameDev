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

    SetSkew(0, 130.0f, 120); // è„ï”Ç40pxâEÇ…Ç∏ÇÁÇ∑(óví≤êÆ)

    SetConstBuffer(0, 0, { 0.0f, 0.0f, 0.0f, 0.0f });
    SetEnabled(true);
}

void PostEffectGuideUI::Update()
{

}
