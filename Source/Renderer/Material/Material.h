#pragma once
#include <string>
#include <vector>
#include <DxLib.h>

// シェーダー・定数バッファ・テクスチャをまとめて管理するクラス。
// 3Dモデル用（VS+PS）と 2D/PostEffect 用（PSのみ）の両方に対応する。
class Material
{
public:

    // GPU転送スロット番号（DxLib仕様固定値）
    static constexpr int SLOT_VS = 7;   // 頂点シェーダー用
    static constexpr int SLOT_PS = 4;   // ピクセルシェーダー用

    // -----------------------------------------------------------------
    // 3Dモデル用コンストラクタ（VS + PS）
    // -----------------------------------------------------------------
    Material(
        const std::string& vsFile, int constBufSizeVS,
        const std::string& psFile, int constBufSizePS,
        int texSlotNum,
        int texAddress = DX_TEXADDRESS_CLAMP);

    // -----------------------------------------------------------------
    // 2D / PostEffect 用コンストラクタ（PS のみ）
    // -----------------------------------------------------------------
    Material(
        const std::string& psFile, int constBufSizePS,
        int texSlotNum,
        int texAddress = DX_TEXADDRESS_CLAMP);

    virtual ~Material(void);

    // -----------------------------------------------------------------
    // 定数バッファ値の設定（オーバーロードで必要な成分だけ指定できる）
    // 指定しなかった成分は 0.0f になる
    // -----------------------------------------------------------------

    // 頂点シェーダー用
    void SetConstVS(int index, float x);
    void SetConstVS(int index, float x, float y);
    void SetConstVS(int index, float x, float y, float z);
    void SetConstVS(int index, float x, float y, float z, float w);

    // ピクセルシェーダー用
    void SetConstPS(int index, float x);
    void SetConstPS(int index, float x, float y);
    void SetConstPS(int index, float x, float y, float z);
    void SetConstPS(int index, float x, float y, float z, float w);

    // テクスチャの設定
    void SetTexture(int slot, int graphHandle);

    // --- Renderer が参照するゲッター ---
    int GetVSHandle(void)       const;
    int GetPSHandle(void)       const;
    int GetConstBufVSH(void)    const;
    int GetConstBufPSH(void)    const;
    int GetConstBufSizeVS(void) const;
    int GetConstBufSizePS(void) const;
    int GetTexAddress(void)     const;
    const std::vector<FLOAT4>& GetConstsVS(void) const;
    const std::vector<FLOAT4>& GetConstsPS(void) const;
    const std::vector<int>& GetTextures(void) const;

private:

    int vsH_ = -1;
    int psH_ = -1;

    int constBufVSH_ = -1;
    int constBufPSH_ = -1;
    int constBufSizeVS_ = 0;
    int constBufSizePS_ = 0;

    int texAddress_;

    std::vector<FLOAT4> constsVS_;
    std::vector<FLOAT4> constsPS_;
    std::vector<int>    texHs_;

    void InitPS(const std::string& psFile, int constBufSizePS, int texSlotNum);
};