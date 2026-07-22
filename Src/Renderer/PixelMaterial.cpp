#include "PixelMaterial.h"
#include <DxLib.h>

PixelMaterial::PixelMaterial(std::string shaderFileName, int constBufFloat4Size, int texSlotNum, int texAddress)
	:
	constBufFloat4Size_(constBufFloat4Size),
	texAddress_(texAddress)
{
	// ピクセルシェーダーのロード
	shaderH_ = LoadPixelShader(shaderFileName.c_str());

	// ピクセルシェーダー用の定数バッファ(RAM)のハンドルを作成
	shaderConstBufH_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * constBufFloat4Size_);
	// 定数バッファ(RAM)のサイズ初期化
	constBufs_.resize(constBufFloat4Size_);

	// テクスチャスロットの確保サイズ初期化
	texHs_.resize(texSlotNum);
}

PixelMaterial::~PixelMaterial(void)
{
	// シェーダーの解放
	DeleteShader(shaderH_);

	// 定数バッファの解放
	DeleteShaderConstantBuffer(shaderConstBufH_);
}

void PixelMaterial::SetTexture(int index, int graphHandle)
{
	texHs_[index] = graphHandle;
}

void PixelMaterial::SetConstBuffer(int index, const FLOAT4& value)
{
	constBufs_[index] = value;
}
int PixelMaterial::GetShaderHandle(void) const
{
	return shaderH_;
}

int PixelMaterial::GetConstBufferHandle(void) const
{
	return shaderConstBufH_;
}

int PixelMaterial::GetConstBufFloat4Size(void) const
{
	return constBufFloat4Size_;
}

const std::vector<FLOAT4>& PixelMaterial::GetConstBuffers(void) const
{
	return constBufs_;
}

const std::vector<int>& PixelMaterial::GetTextures(void) const
{
	return texHs_;
}

int PixelMaterial::GetTexAddress(void) const
{
	return texAddress_;
}

