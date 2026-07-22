#pragma once
#include <string>
#include <vector>
#include <map>
#include <DxLib.h>

class ModelMaterial
{

public:

	// 設定できる最大のテクスチャの数
	static constexpr int MAX_TEXTURES = 4;

	// テクスチャアドレスタイプ
	enum class TEXADDRESS
	{
		NONE = 0,
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MAX
	};

	/// <summary>
	/// コンストラクタ（3Dモデル用）
	/// </summary>
	/// <param name="shaderFileName">頂点シェーダーファイル名</param>
	/// <param name="constBufFloat4Size">頂点シェーダ用定数バッファの確保サイズ(FLOAT4をいくつ作るか)</param>
	/// /// <param name="shaderFileName">ピクセルシェーダーファイル名</param>
	/// <param name="constBufFloat4Size">ピクセルシェーダ用定数バッファの確保サイズ(FLOAT4をいくつ作るか)</param>
	ModelMaterial(std::string fileNameVS, int constBufFloat4SizeVS,
				  std::string fileNamePS, int constBufFloat4SizePS);

	/// <summary>
	/// コンストラクタ（ポストエフェクト用）
	/// </summary>
	/// <param name="fileNamePS"></param>
	/// <param name="constBufFloat4SizePS"></param>
	ModelMaterial(std::string fileNamePS, int constBufFloat4SizePS);

	// デストラクタ
	~ModelMaterial(void);

	// 定数バッファを追加
	void AddConstBufVS(const FLOAT4& constBuf); // 頂点
	void AddConstBufPS(const FLOAT4& constBuf); // ピクセル

	// 定数バッファを更新
	void SetConstBufVS(int idx, const FLOAT4& constBuf); // 頂点
	void SetConstBufPS(int idx, const FLOAT4& constBuf); // ピクセル

	// テクスチャ追加
	void AddTextureBuf(int texDiffuse);
	// テクスチャ更新
	void SetTextureBuf(int idx, int texDiffuse);

	// シェーダハンドル取得
	int GetShaderVSH(void) const; // 頂点
	int GetShaderPSH(void) const; // ピクセル

	// 定数バッファハンドル取得
	int GetConstBufVSH(void) const; // 頂点
	int GetConstBufPSH(void) const; // ピクセル

	// 定数バッファ
	const std::vector<FLOAT4>& GetConstBufsVS(void) const; // 頂点
	const std::vector<FLOAT4>& GetConstBufsPS(void) const; // ピクセル

	// テクスチャ取得
	const std::map<int, int>& GetTextures(void) const;

	// テクスチャアドレスを取得
	TEXADDRESS GetTextureAddress(void) const;

	// テクスチャアドレスを設定
	void SetTextureAddress(TEXADDRESS texA);

private:

	// シェーダハンドル
	int shaderVSH_; // 頂点
	int shaderPSH_; // ピクセル

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)
	int constBufFloat4SizeVS_; // 頂点用
	int constBufFloat4SizePS_; // ピクセル用

	// 定数バッファハンドル
	int constBufVSH_; // 頂点
	int constBufPSH_; // ピクセル

	// テクスチャアドレス
	TEXADDRESS texAddress_;

	// 定数バッファ
	std::vector<FLOAT4> constBufsVS_; // 頂点
	std::vector<FLOAT4> constBufsPS_; // ピクセル

	// 画像
	std::map<int, int> textures_;

};
