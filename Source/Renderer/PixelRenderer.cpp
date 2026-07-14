#include <DxLib.h>
#include "Material/Material.h"
#include "PixelRenderer.h"

PixelRenderer::PixelRenderer(Material& material)
    : material_(material)
{
}

void PixelRenderer::MakeSquareVertex(Vector2 size, Vector2 pos)
{
    size_ = size;
    pos_ = pos;

    float sX = static_cast<float>(pos_.x);
    float sY = static_cast<float>(pos_.y);
    float eX = static_cast<float>(pos_.x + size_.x);
    float eY = static_cast<float>(pos_.y + size_.y);

    for (int i = 0; i < NUM_VERTEX; i++)
    {
        vertexs_[i].rhw = 1.0f;
        vertexs_[i].dif = GetColorU8(255, 255, 255, 255);
        vertexs_[i].spc = GetColorU8(0, 0, 0, 255);
        vertexs_[i].su = 0.0f;
        vertexs_[i].sv = 0.0f;
    }

    // ¶ã
    vertexs_[0].pos = VGet(sX, sY, 0.0f);
    vertexs_[0].u = 0.0f; vertexs_[0].v = 0.0f;
    // ‰Eã
    vertexs_[1].pos = VGet(eX, sY, 0.0f);
    vertexs_[1].u = 1.0f; vertexs_[1].v = 0.0f;
    // ‰E‰º
    vertexs_[2].pos = VGet(eX, eY, 0.0f);
    vertexs_[2].u = 1.0f; vertexs_[2].v = 1.0f;
    // ¶‰º
    vertexs_[3].pos = VGet(sX, eY, 0.0f);
    vertexs_[3].u = 0.0f; vertexs_[3].v = 1.0f;

    MakeVertexIndex();
}

void PixelRenderer::MakeVertexIndex(void)
{
    //  0---1
    //  |  /|
    //  | / |
    //  3---2

    indexes_[0] = 0; indexes_[1] = 1; indexes_[2] = 3;
    indexes_[3] = 1; indexes_[4] = 2; indexes_[5] = 3;
}

void PixelRenderer::Draw(void)
{
    SetToDevice();
    DrawPolygonIndexed2DToShader(vertexs_, NUM_VERTEX, indexes_, NUM_POLYGON);
    Reset();
}

void PixelRenderer::SetToDevice(void)
{
    MV1SetUseOrigShader(true);

    SetUsePixelShader(material_.GetPSHandle());
    SetTextureAddressMode(material_.GetTexAddress());

    const auto& textures = material_.GetTextures();
    for (int i = 0; i < (int)textures.size(); i++)
    {
        SetUseTextureToShader(i, textures[i]);
    }

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
}

void PixelRenderer::Reset(void)
{
    const auto& textures = material_.GetTextures();
    for (int i = 0; i < (int)textures.size(); i++)
    {
        SetUseTextureToShader(i, -1);
    }
    SetUsePixelShader(-1);
    MV1SetUseOrigShader(false);
}