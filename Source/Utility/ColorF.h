// Core/ColorF.h
#pragma once
#include <string>

struct Color;

// カラー（0.0f?1.0f）
struct ColorF
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;  // デフォルトは不透明

    // コンストラクタ
    ColorF(void) = default;
    ColorF(float r, float g, float b);
    ColorF(float r, float g, float b, float a);

    // 演算子オーバーロード
    ColorF  operator+ (const ColorF& other) const;
    ColorF  operator* (float scalar)         const;
    ColorF& operator+=(const ColorF& other);
    ColorF& operator*=(float scalar);
    bool    operator==(const ColorF& other)  const;
    bool    operator!=(const ColorF& other)  const;

    // 線形補間（t: 0.0f?1.0f）
    ColorF Lerp(const ColorF& other, float t) const;

    // Color との変換
    Color ToColor(void) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // 静的メソッド
    static ColorF Lerp(const ColorF& a, const ColorF& b, float t);

    // 基本色定数
    static constexpr ColorF WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
    static constexpr ColorF BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
    static constexpr ColorF RED = { 1.0f, 0.0f, 0.0f, 1.0f };
    static constexpr ColorF GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
    static constexpr ColorF BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
    static constexpr ColorF YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
    static constexpr ColorF CYAN = { 0.0f, 1.0f, 1.0f, 1.0f }; // 青緑
    static constexpr ColorF MAGENTA = { 1.0f, 0.0f, 1.0f, 1.0f }; // 赤紫
    static constexpr ColorF ORANGE = { 1.0f, 0.5f, 0.0f, 1.0f }; // 橙
    static constexpr ColorF PURPLE = { 0.5f, 0.0f, 0.5f, 1.0f }; // 紫
    static constexpr ColorF PINK = { 1.0f, 0.41f,0.71f,1.0f }; // 桃色
    static constexpr ColorF BROWN = { 0.55f,0.27f,0.07f,1.0f }; // 茶色
    static constexpr ColorF GRAY = { 0.5f, 0.5f, 0.5f, 1.0f }; // 中間グレー
    static constexpr ColorF SILVER = { 0.75f,0.75f,0.75f,1.0f }; // 明るいグレー

    // UI用定数
    static constexpr ColorF HP_HIGH = { 0.0f, 1.0f, 0.0f, 1.0f }; // HP高い
    static constexpr ColorF HP_MID = { 1.0f, 1.0f, 0.0f, 1.0f }; // HP中
    static constexpr ColorF HP_LOW = { 1.0f, 0.0f, 0.0f, 1.0f }; // HP低い

    // エフェクト用定数
    static constexpr ColorF DAMAGE = { 1.0f, 0.27f,0.27f,1.0f }; // ダメージ
};