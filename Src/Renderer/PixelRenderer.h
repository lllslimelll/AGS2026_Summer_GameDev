#pragma once
#include <DxLib.h>
#include "PixelMaterial.h"
#include "../Common/Vector2.h"

class PixelRenderer
{
public:
	/// <summary>
	///	コンストラクタ
	/// </summary>
	/// <param name="material">マテリアル</param>
	PixelRenderer(PixelMaterial& material);

	/// <summary>
	///	描画矩形の生成
	/// </summary>
	/// <param name="size">描画矩形のサイズ</param>
	/// <param name="pos">描画矩形の座標</param>
	void MakeSquareVertex(Vector2 size, Vector2 pos);

	// 頂点インデックスの生成
	void MakeVertexIndex(void);
	void MakeSkewVertex(Vector2 size, Vector2 pos, float skewX);

	// 描画
	void Draw(void);

private:

	// 頂点数
	static constexpr int NUM_VERTEX = 4;
	// 頂点インデックス数
	static constexpr int NUM_VERTEX_INDEX = 6;

	// ポリゴン数
	static constexpr int NUM_POLYGON = 2;

	// マテリアル
	PixelMaterial& material_;

	// 座標
	Vector2 pos_;

	// 描画サイズ
	Vector2 size_;

	// 頂点
	VERTEX2DSHADER vertexs_[NUM_VERTEX];

	// 頂点インデックス
	WORD indexes_[NUM_VERTEX_INDEX];

	// マテリアルの設定をGPUに反映
	void SetToDevice();
	// マテリアルの設定をリセット
	void Reset();
};

