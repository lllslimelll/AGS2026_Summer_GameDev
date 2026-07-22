#pragma once
#include <DxLib.h>
#include "../Common/Vector2.h"
#include "../Renderer/ModelMaterial.h"
#include "../Renderer/PixelMaterial.h"

class ModelRenderer
{

public:

	// シェーダ用オリジナル定数バッファの使用開始スロット
	static constexpr int CONSTANT_BUF_SLOT_BEGIN_VS = 7; // 頂点用
	static constexpr int CONSTANT_BUF_SLOT_BEGIN_PS = 4; // ピクセル用

	// コンストラクタ
	ModelRenderer(ModelMaterial& modelMaterial, int& modelH);

	// デストラクタ
	~ModelRenderer(void);

	// 描画
	void Draw(void);

private:

	// モデルマテリアル
	ModelMaterial& modelMaterial_; 

	// モデルのハンドル
	int& modelH_;

	// シェーダ設定（頂点）
	void SetReserveVS(void);
	// シェーダ設定（ピクセル）
	void SetReservePS(void);
};
