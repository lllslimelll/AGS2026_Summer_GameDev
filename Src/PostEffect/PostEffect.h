#pragma once
#include <vector>
#include <string>
#include <memory>
#include <DxLib.h>

class PixelMaterial;
class PixelRenderer;

class PostEffect
{
public:

	/// コンストラクタ
	PostEffect(void);

	// デストラクタ
	virtual ~PostEffect(void);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="targetScreen">ポストエフェクトをかけるスクリーン</param>
	virtual void Init(int targetScreen);

	// 更新
	virtual void Update(void) {};

	// 描画
	void Draw(void);

	/// <summary>
	/// ポストエフェクト適用設定
	/// </summary>
	/// <param name="enabled">適用フラグ</param>
	void SetEnabled(bool enabled);

protected:

	// エフェクト追加
	virtual void InitEffect(void) = 0;

	/// <summary>
	/// エフェクト追加
	/// </summary>
	/// <param name="shaderFileName">シェーダーファイル名</param>
	/// <param name="constBufFloat4Size">定数バッファのサイズ(FLOAT4の個数)</param>
	/// <param name="texSlotNum">テクスチャのスロット数</param>
	/// /// <param name="texAdress">テクスチャアドレス(デフォルト:クランプ)</param>
	void Add(std::string shaderFileName, int constBufFloat4Size, int texSlotNum,
		int texAddress = DX_TEXADDRESS_CLAMP);

	/// <summary>
	/// 定数バッファの値をセット
	/// </summary>
	/// <param name="effectIndex">エフェクトのインデックス</param>
	/// <param name="bufIndex">定数バッファのインデックス</param>
	/// <param name="value">設定する値</param>
	void SetConstBuffer(int effectIndex, int bufIndex, const FLOAT4& value);

	void SetSkew(int rendererIndex, float skewX, int offsetY = 0);

	//private:

	// マテリアルのリスト
	std::vector<std::unique_ptr<PixelMaterial>> materials_;
	// レンダラーのリスト
	std::vector <std::unique_ptr <PixelRenderer>> renderers_;

	// 2枚のスクリーン(ピンポンバッファ用)
	int pingPongScreens_[2];

	// 対象スクリーン
	int targetScreen_;

	// ポストエフェクト適用フラグ
	bool enabled_;
};

