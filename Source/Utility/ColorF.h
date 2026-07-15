// Core/ColorF.h
#pragma once
#include <string>

// カラー
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

    // 線形補間
    ColorF Lerp(const ColorF& other, float t) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // 静的メソッド
    static ColorF Lerp(const ColorF& a, const ColorF& b, float t);
    
    // 基本色定数
    static const ColorF WHITE;
    static const ColorF BLACK;
    static const ColorF RED;
    static const ColorF GREEN;
    static const ColorF BLUE;
    static const ColorF YELLOW;
    static const ColorF CYAN;    // 青緑
    static const ColorF MAGENTA; // 赤紫
    static const ColorF ORANGE;  // 橙
    static const ColorF PURPLE;  // 紫
    static const ColorF PINK;    // 桃色
    static const ColorF BROWN;   // 茶色
    static const ColorF GRAY;    // 中間グレー
    static const ColorF SILVER;  // 明るいグレー

    // UI用定数
    static const ColorF HP_HIGH; // HP高い
    static const ColorF HP_MID;  // HP中
    static const ColorF HP_LOW;  // HP低い

    // エフェクト用定数
    static const ColorF DAMAGE;  // ダメージ
};