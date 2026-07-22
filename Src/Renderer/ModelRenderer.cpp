#include "ModelRenderer.h"

ModelRenderer::ModelRenderer(ModelMaterial& modelMaterial, int& modelH)
	:
	modelMaterial_(modelMaterial),
	modelH_(modelH)
{
}

ModelRenderer::~ModelRenderer()
{
}

void ModelRenderer::Draw(void)
{
	// オリジナルシェーダ設定（ON）
	MV1SetUseOrigShader(true);

	// シェーダ設定
	SetReserveVS(); // 頂点
	SetReservePS(); // ピクセル

	// テクスチャアドレスタイプ取得
	auto texA = modelMaterial_.GetTextureAddress();
	int texAType = static_cast<int>(texA);

	// テクスチャアドレスタイプを変更
	SetTextureAddressModeUV(texAType, texAType);

	// 描画
	MV1DrawModel(modelH_);

	// テクスチャアドレスタイプを元に戻す
	SetTextureAddressModeUV(DX_TEXADDRESS_CLAMP, DX_TEXADDRESS_CLAMP);

	// 後始末
	//-----------------------------------------

	// テクスチャ解除
	const auto& textures = modelMaterial_.GetTextures();
	size_t size = textures.size();
	if (size == 0)
	{
		// 前回使用分のテクスチャを引き継がないように
		SetUseTextureToShader(0, -1);
	}
	else
	{
		for (const auto& pair : textures)
		{
			SetUseTextureToShader(pair.first, -1);
		}
	}

	// シェーダ解除
	SetUseVertexShader(-1); // 頂点
	SetUsePixelShader(-1);  // ピクセル

	// オリジナルシェーダ設定(OFF)
	MV1SetUseOrigShader(false);
	//-----------------------------------------

}

void ModelRenderer::SetReserveVS(void)
{
	// 定数バッファハンドル取得
	int constBuf = modelMaterial_.GetConstBufVSH();

	if (constBuf != -1)
	{
		FLOAT4* constBufsPtr = (FLOAT4*)GetBufferShaderConstantBuffer(constBuf);
		const auto& constBufs = modelMaterial_.GetConstBufsVS();

		size_t size = constBufs.size();
		for (int i = 0; i < size; i++)
		{
			if (i != 0)
			{
				constBufsPtr++;
			}
			constBufsPtr->x = constBufs[i].x;
			constBufsPtr->y = constBufs[i].y;
			constBufsPtr->z = constBufs[i].z;
			constBufsPtr->w = constBufs[i].w;
		}

		// 頂点シェーダー用の定数バッファを更新して書き込んだ内容を反映する
		UpdateShaderConstantBuffer(constBuf);

		// 頂点シェーダー用の定数バッファを定数バッファレジスタにセット
		SetShaderConstantBuffer(
			constBuf, DX_SHADERTYPE_VERTEX, CONSTANT_BUF_SLOT_BEGIN_VS);
	}

	// 頂点シェーダー設定
	SetUseVertexShader(modelMaterial_.GetShaderVSH());
}

void ModelRenderer::SetReservePS(void)
{
	// ピクセルシェーダにテクスチャを転送
	const auto& textures = modelMaterial_.GetTextures();
	size_t size = textures.size();
	if (size == 0)
	{
		// 前回使用分のテクスチャを引き継がないように
		SetUseTextureToShader(0, -1);
	}
	else
	{
		for (const auto& pair : textures)
		{
			SetUseTextureToShader(pair.first, pair.second);
		}
	}

	// 定数バッファハンドル取得
	int constBuf = modelMaterial_.GetConstBufPSH();

	if (constBuf != -1)
	{
		FLOAT4* constBufsPtr = (FLOAT4*)GetBufferShaderConstantBuffer(constBuf);
		const auto& constBufs = modelMaterial_.GetConstBufsPS();

		size = constBufs.size();
		for (int i = 0; i < size; i++)
		{
			if (i != 0)
			{
				constBufsPtr++;
			}
			constBufsPtr->x = constBufs[i].x;
			constBufsPtr->y = constBufs[i].y;
			constBufsPtr->z = constBufs[i].z;
			constBufsPtr->w = constBufs[i].w;
		}

		// ピクセルシェーダー用の定数バッファを更新して書き込んだ内容を反映する
		UpdateShaderConstantBuffer(constBuf);

		// ピクセルシェーダー用の定数バッファを定数バッファレジスタにセット
		SetShaderConstantBuffer(
			constBuf, DX_SHADERTYPE_PIXEL, CONSTANT_BUF_SLOT_BEGIN_PS);
	}

	// ピクセルシェーダー設定
	SetUsePixelShader(modelMaterial_.GetShaderPSH());
}
