#include "../Application.h"
#include "ModelMaterial.h"

ModelMaterial::ModelMaterial(std::string fileNameVS, int constBufFloat4SizeVS,
							 std::string fileNamePS, int constBufFloat4SizePS)
{
	// 頂点シェーダのロード
	shaderVSH_ = LoadVertexShader(
		(Application::PATH_SHADER + fileNameVS).c_str());
	// ピクセルシェーダのロード
	shaderPSH_ = LoadPixelShader(
		(Application::PATH_SHADER + fileNamePS).c_str());

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)
	constBufFloat4SizeVS_ = constBufFloat4SizeVS; // 頂点用
	constBufFloat4SizePS_ = constBufFloat4SizePS; // ピクセル用

	constBufVSH_ = -1;
	constBufPSH_ = -1;
	// シェーダー用の定数バッファを作成
	// 0ガード
	if (constBufFloat4SizeVS_ > 0)
	{
		constBufVSH_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * constBufFloat4SizeVS_); // 頂点
	}
	if (constBufFloat4SizePS_ > 0)
	{
		constBufPSH_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * constBufFloat4SizePS_); // ピクセル
	}

	// テクスチャアドレス
	texAddress_ = TEXADDRESS::CLAMP;
}

ModelMaterial::ModelMaterial(std::string fileNamePS, int constBufFloat4SizePS)
{
}


void ModelMaterial::AddConstBufVS(const FLOAT4& constBuf)
{
	if (constBufFloat4SizeVS_ > constBufsVS_.size())
	{
		constBufsVS_.emplace_back(constBuf);
	}
}

void ModelMaterial::AddConstBufPS(const FLOAT4& constBuf)
{
	if (constBufFloat4SizePS_ > constBufsPS_.size())
	{
		constBufsPS_.emplace_back(constBuf);
	}
}

void ModelMaterial::SetConstBufVS(int idx, const FLOAT4& constBuf)
{
	if (idx >= constBufsVS_.size())
	{
		return;
	}

	constBufsVS_[idx] = constBuf;
}

void ModelMaterial::SetConstBufPS(int idx, const FLOAT4& constBuf)
{
	if (idx >= constBufsPS_.size())
	{
		return;
	}

	constBufsPS_[idx] = constBuf;
}

void ModelMaterial::AddTextureBuf(int texDiffuse)
{
	// 現在のサイズをスロット番号として使う
	// 0番はモデルのデフォルトテクスチャ用なので1番から
	int slot = textures_.size() + 1;
	textures_[slot] = texDiffuse;
}

void ModelMaterial::SetTextureBuf(int idx, int texDiffuse)
{

	if (idx >= textures_.size())
	{
		return;
	}

	textures_[idx] = texDiffuse;

}

int ModelMaterial::GetShaderVSH(void) const
{
	return shaderVSH_;
}

int ModelMaterial::GetShaderPSH(void) const
{
	return shaderPSH_;
}

int ModelMaterial::GetConstBufVSH(void) const
{
	return constBufVSH_;
}

int ModelMaterial::GetConstBufPSH(void) const
{
	return constBufPSH_;
}

const std::vector<FLOAT4>& ModelMaterial::GetConstBufsVS(void) const
{
	return constBufsVS_;
}

const std::vector<FLOAT4>& ModelMaterial::GetConstBufsPS(void) const
{
	return constBufsPS_;
}

const std::map<int, int>& ModelMaterial::GetTextures(void) const
{
	return textures_;
}

ModelMaterial::TEXADDRESS ModelMaterial::GetTextureAddress(void) const
{
	return texAddress_;
}

void ModelMaterial::SetTextureAddress(TEXADDRESS texA)
{
	texAddress_ = texA;
}

ModelMaterial::~ModelMaterial(void)
{
	DeleteShader(shaderVSH_);
	DeleteShader(shaderPSH_);
	DeleteShaderConstantBuffer(constBufVSH_);
	DeleteShaderConstantBuffer(constBufPSH_);
}
