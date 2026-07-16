#include <DxLib.h>
#include "../../Application.h"
#include "Material.h"

// ---------------------------------------------------------------
// 3Dモデル用コンストラクタ（VS + PS）
// ---------------------------------------------------------------
Material::Material(
    const std::string& vsFile, int constBufSizeVS,
    const std::string& psFile, int constBufSizePS,
    int texSlotNum,
    int texAddress)
    :
    constBufSizeVS_(constBufSizeVS),
    constBufSizePS_(constBufSizePS),
    texAddress_(texAddress)
{
    vsH_ = LoadVertexShader(
        (Application::PATH_SHADER + vsFile).c_str());

    if (constBufSizeVS_ > 0)
    {
        constBufVSH_ = CreateShaderConstantBuffer(
            sizeof(FLOAT4) * constBufSizeVS_);
        constsVS_.resize(constBufSizeVS_, { 0.0f, 0.0f, 0.0f, 0.0f });
    }

    InitPS(psFile, constBufSizePS, texSlotNum);
}

// ---------------------------------------------------------------
// 2D / PostEffect 用コンストラクタ（PS のみ）
// ---------------------------------------------------------------
Material::Material(
    const std::string& psFile, int constBufSizePS,
    int texSlotNum,
    int texAddress)
    : constBufSizeVS_(0),
    constBufSizePS_(constBufSizePS),
    texAddress_(texAddress)
{
    InitPS(psFile, constBufSizePS, texSlotNum);
}

// ---------------------------------------------------------------
// デストラクタ
// ---------------------------------------------------------------
Material::~Material(void)
{
    if (vsH_ != -1) DeleteShader(vsH_);
    if (psH_ != -1) DeleteShader(psH_);
    if (constBufVSH_ != -1) DeleteShaderConstantBuffer(constBufVSH_);
    if (constBufPSH_ != -1) DeleteShaderConstantBuffer(constBufPSH_);
}

// ---------------------------------------------------------------
// PS ロード・バッファ生成の共通処理
// ---------------------------------------------------------------
void Material::InitPS(
    const std::string& psFile, int constBufSizePS, int texSlotNum)
{
    psH_ = LoadPixelShader(
        (Application::PATH_SHADER + psFile).c_str());

    if (constBufSizePS > 0)
    {
        constBufPSH_ = CreateShaderConstantBuffer(
            sizeof(FLOAT4) * constBufSizePS);
        constsPS_.resize(constBufSizePS, { 0.0f, 0.0f, 0.0f, 0.0f });
    }

    texHs_.resize(texSlotNum, -1);
}

// ---------------------------------------------------------------
// 定数バッファ値の設定（VS）
// 指定しなかった成分は 0.0f
// ---------------------------------------------------------------
void Material::SetConstVS(int index, float x)
{
    SetConstVS(index, x, 0.0f, 0.0f, 0.0f);
}
void Material::SetConstVS(int index, float x, float y)
{
    SetConstVS(index, x, y, 0.0f, 0.0f);
}
void Material::SetConstVS(int index, float x, float y, float z)
{
    SetConstVS(index, x, y, z, 0.0f);
}
void Material::SetConstVS(int index, float x, float y, float z, float w)
{
    if (index >= 0 && index < (int)constsVS_.size())
    {
        constsVS_[index] = { x, y, z, w };
    }
}

// ---------------------------------------------------------------
// 定数バッファ値の設定（PS）
// 指定しなかった成分は 0.0f
// ---------------------------------------------------------------
void Material::SetConstPS(int index, float x)
{
    SetConstPS(index, x, 0.0f, 0.0f, 0.0f);
}
void Material::SetConstPS(int index, float x, float y)
{
    SetConstPS(index, x, y, 0.0f, 0.0f);
}
void Material::SetConstPS(int index, float x, float y, float z)
{
    SetConstPS(index, x, y, z, 0.0f);
}
void Material::SetConstPS(int index, float x, float y, float z, float w)
{
    if (index >= 0 && index < (int)constsPS_.size())
    {
        constsPS_[index] = { x, y, z, w };
    }
}

// ---------------------------------------------------------------
// テクスチャの設定
// ---------------------------------------------------------------
void Material::SetTexture(int slot, int graphHandle)
{
    if (slot >= 0 && slot < (int)texHs_.size())
    {
        texHs_[slot] = graphHandle;
    }
}

// ---------------------------------------------------------------
// ゲッター
// ---------------------------------------------------------------
int Material::GetVSHandle(void)       const { return vsH_; }
int Material::GetPSHandle(void)       const { return psH_; }
int Material::GetConstBufVSH(void)    const { return constBufVSH_; }
int Material::GetConstBufPSH(void)    const { return constBufPSH_; }
int Material::GetConstBufSizeVS(void) const { return constBufSizeVS_; }
int Material::GetConstBufSizePS(void) const { return constBufSizePS_; }
int Material::GetTexAddress(void)     const { return texAddress_; }

const std::vector<FLOAT4>& Material::GetConstsVS(void) const
{
    return constsVS_;
}
const std::vector<FLOAT4>& Material::GetConstsPS(void) const
{
    return constsPS_;
}
const std::vector<int>& Material::GetTextures(void) const
{
    return texHs_;
}