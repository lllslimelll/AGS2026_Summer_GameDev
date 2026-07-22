#pragma once
#include <string>
#include <vector>
#include <DxLib.h>

class PixelMaterial
{
public:

	// ピクセルシェーダ用オリジナル定数バッファの使用開始スロット
	static constexpr int CONSTANT_BUF_SLOT_BEGIN_PS = 4;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="shaderFileName">シェーダーファイル名</param>
	/// <param name="constBufFloat4Size">定数バッファの確保サイズ</param>
	/// <param name="textureSize">テクスチャスロットの確保サイズ(デフォルト:クランプ)</param>
	///	<param name="textureSize">テクスチャアドレス</param>
	PixelMaterial(std::string shaderFileName,
		int constBufFloat4Size,
		int texSlotNum,
		int texAddress);

	// デストラクタ
	~PixelMaterial(void);

	// テクスチャを設定
	void SetTexture(int index, int graphHandle);

	// 定数バッファに値をセットする
	void SetConstBuffer(int index, const FLOAT4& value);

	// シェーダハンドルを取得
	int GetShaderHandle(void) const;

	// 定数バッファハンドルを取得
	int GetConstBufferHandle(void) const;

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)を取得
	int GetConstBufFloat4Size(void) const;

	// テクスチャアドレスを取得
	int GetTexAddress(void) const;

	// 定数バッファ(RAM)のリストを取得
	const std::vector<FLOAT4>& GetConstBuffers(void) const;

	// テクスチャハンドルのリストを取得
	const std::vector<int>& GetTextures(void) const;

private:

	// シェーダハンドル
	int shaderH_;

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)
	int constBufFloat4Size_;

	// 定数バッファハンドル
	int shaderConstBufH_;

	// テクスチャアドレス
	int texAddress_;

	// 定数バッファ
	std::vector<FLOAT4> constBufs_;

	// テクスチャハンドルのリスト
	std::vector<int> texHs_;
};

