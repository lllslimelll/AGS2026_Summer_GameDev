#include <DxLib.h>
#include "Material/Material.h"
#include "ModelRenderer.h"

ModelRenderer::ModelRenderer(Material& material, int& modelH)
    : material_(material), modelH_(modelH)
{
}

ModelRenderer::~ModelRenderer(void)
{
}

void ModelRenderer::Draw(void)
{
    SetToDevice();
    MV1DrawModel(modelH_);
    Reset();
}

void ModelRenderer::SetToDevice(void)
{
    MV1SetUseOrigShader(true);

    SetTextureAddressModeUV(
        material_.GetTexAddress(),
        material_.GetTexAddress());

    // テクスチャを GPU に転送
    const auto& textures = material_.GetTextures();
    for (int i = 0; i < (int)textures.size(); i++)
    {
        SetUseTextureToShader(i, textures[i]);
    }

    // 頂点シェーダー
    int constBufVSH = material_.GetConstBufVSH();
    if (constBufVSH != -1)
    {
        FLOAT4* ptr = (FLOAT4*)GetBufferShaderConstantBuffer(constBufVSH);
        const auto& constsVS = material_.GetConstsVS();
        for (int i = 0; i < material_.GetConstBufSizeVS(); i++)
        {
            ptr[i] = constsVS[i];
        }
        UpdateShaderConstantBuffer(constBufVSH);
        SetShaderConstantBuffer(
            constBufVSH, DX_SHADERTYPE_VERTEX, Material::SLOT_VS);
    }
    SetUseVertexShader(material_.GetVSHandle());

    // ピクセルシェーダー
    int constBufPSH = material_.GetConstBufPSH();
    if (constBufPSH != -1)
    {
        FLOAT4* ptr = (FLOAT4*)GetBufferShaderConstantBuffer(constBufPSH);
        const auto& constsPS = material_.GetConstsPS();
        for (int i = 0; i < material_.GetConstBufSizePS(); i++)
        {
            ptr[i] = constsPS[i];
        }
        UpdateShaderConstantBuffer(constBufPSH);
        SetShaderConstantBuffer(
            constBufPSH, DX_SHADERTYPE_PIXEL, Material::SLOT_PS);
    }
    SetUsePixelShader(material_.GetPSHandle());
}

void ModelRenderer::Reset(void)
{
    const auto& textures = material_.GetTextures();
    if (textures.empty())
    {
        SetUseTextureToShader(0, -1);
    }
    else
    {
        for (int i = 0; i < (int)textures.size(); i++)
        {
            SetUseTextureToShader(i, -1);
        }
    }

    SetUseVertexShader(-1);
    SetUsePixelShader(-1);
    SetTextureAddressModeUV(DX_TEXADDRESS_CLAMP, DX_TEXADDRESS_CLAMP);
    MV1SetUseOrigShader(false);
}