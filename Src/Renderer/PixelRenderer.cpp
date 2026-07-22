#include "../Application.h"
#include "PixelRenderer.h"


PixelRenderer::PixelRenderer(PixelMaterial& material)
	:
	material_(material)
{
}

void PixelRenderer::MakeSquareVertex(Vector2 size, Vector2 pos)
{
	size_ = size;
	pos_ = pos;

	int cnt = 0;
	float sX = static_cast<float>(pos_.x);
	float sY = static_cast<float>(pos_.y);
	float eX = static_cast<float>(pos_.x + size_.x);
	float eY = static_cast<float>(pos_.y + size_.y);

	// ４頂点の初期化
	for (int i = 0; i < 4; i++)
	{
		vertexs_[i].rhw = 1.0f;
		vertexs_[i].dif = GetColorU8(255, 255, 255, 255);
		vertexs_[i].spc = GetColorU8(0, 0, 0, 255);
		vertexs_[i].su = 0.0f;
		vertexs_[i].sv = 0.0f;
	}

	// 左上
	vertexs_[cnt].pos = VGet(sX, sY, 0.0f);
	vertexs_[cnt].u = 0.0f;
	vertexs_[cnt].v = 0.0f;
	cnt++;

	// 右上
	vertexs_[cnt].pos = VGet(eX, sY, 0.0f);
	vertexs_[cnt].u = 1.0f;
	vertexs_[cnt].v = 0.0f;
	cnt++;

	// 右下
	vertexs_[cnt].pos = VGet(eX, eY, 0.0f);
	vertexs_[cnt].u = 1.0f;
	vertexs_[cnt].v = 1.0f;
	cnt++;

	// 左下
	vertexs_[cnt].pos = VGet(sX, eY, 0.0f);
	vertexs_[cnt].u = 0.0f;
	vertexs_[cnt].v = 1.0f;

	// 頂点インデックスの生成
	MakeVertexIndex();
}

void PixelRenderer::MakeVertexIndex(void)
{
	// 頂点インデックス
	int cnt = 0;
	indexes_[cnt++] = 0;
	indexes_[cnt++] = 1;
	indexes_[cnt++] = 3;

	indexes_[cnt++] = 1;
	indexes_[cnt++] = 2;
	indexes_[cnt++] = 3;
}

void PixelRenderer::Draw(void)
{
	// マテリアルの設定をGPUに反映
	SetToDevice();

	// 描画
	DrawPolygonIndexed2DToShader(vertexs_, NUM_VERTEX, indexes_, NUM_POLYGON);

	// シェーダの後始末
	Reset();
}

void PixelRenderer::SetToDevice(void)
{
	// オリジナルシェーダ設定(ON)
	MV1SetUseOrigShader(true);
	// 使用するピクセルシェーダをGPUに指定
	SetUsePixelShader(material_.GetShaderHandle());
	// テクスチャアドレスモードを設定
	SetTextureAddressMode(material_.GetTexAddress());

	// テクスチャハンドル(RAM)をGPUに指定
	const auto& textures = material_.GetTextures(); // テクスチャの数
	for (int i = 0; i < (int)textures.size(); i++)
	{
		SetUseTextureToShader(i, textures[i]);
	}

	// DxLibが管理するRAM上の定数バッファのポインタを取得
	int constBufH = material_.GetConstBufferHandle(); // 定数バッファのハンドル
	FLOAT4* constBufsPtr = (FLOAT4*)GetBufferShaderConstantBuffer(constBufH);

	// 作成した定数バッファ(RAM)をDxLibが管理する定数バッファ(RAM)にコピー
	const auto& constBufs = material_.GetConstBuffers(); // 定数バッファのリスト
	for (int i = 0; i < material_.GetConstBufFloat4Size(); i++)
	{
		constBufsPtr[i] = constBufs[i];
	}

	// DxLibが管理する定数バッファ(RAM)を更新した内容を定数バッファ(VRAM)に転送
	UpdateShaderConstantBuffer(constBufH);

	// DxLibが管理する定数バッファ(RAM)をピクセルシェーダー用定数バッファレジスタ(VRAM)にセット
	SetShaderConstantBuffer(
		constBufH, DX_SHADERTYPE_PIXEL, PixelMaterial::CONSTANT_BUF_SLOT_BEGIN_PS);
}

void PixelRenderer::Reset(void)
{
	const auto& textures = material_.GetTextures(); // テクスチャの数

	// テクスチャ解除
	for (int i = 0; i < (int)textures.size(); i++)
	{
		SetUseTextureToShader(i, -1);
	}

	// ピクセルシェーダ解除
	SetUsePixelShader(-1);
	// オリジナルシェーダ設定(OFF)
	MV1SetUseOrigShader(false);
}
