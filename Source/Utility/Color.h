#pragma once
#include <string>

struct ColorF;

// DxLib描画用カラー（0xRRGGBB / 0xAARRGGBB）
// Unreal の FColor に相当
struct Color
{
    unsigned int value = 0x000000;

    // コンストラクタ
    Color(void) = default;
    Color(unsigned int value);
    Color(unsigned char r, unsigned char g, unsigned char b);
    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    // 各成分の取得
    unsigned char R(void) const;
    unsigned char G(void) const;
    unsigned char B(void) const;
    unsigned char A(void) const;

    // ColorF との変換
    ColorF ToColorF(void) const;

    // 演算子オーバーロード
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // 基本色定数
    static constexpr Color WHITE = { 0xFFFFFF };
    static constexpr Color BLACK = { 0x000000 };
    static constexpr Color RED = { 0xFF0000 };
    static constexpr Color GREEN = { 0x00FF00 };
    static constexpr Color BLUE = { 0x0000FF };
    static constexpr Color YELLOW = { 0xFFFF00 };
    static constexpr Color CYAN = { 0x00FFFF }; // 青緑
    static constexpr Color MAGENTA = { 0xFF00FF }; // 赤紫
    static constexpr Color ORANGE = { 0xFF8000 }; // 橙
    static constexpr Color PURPLE = { 0x800080 }; // 紫
    static constexpr Color PINK = { 0xFF69B4 }; // 桃色
    static constexpr Color BROWN = { 0x8B4513 }; // 茶色
    static constexpr Color GRAY = { 0x808080 }; // 中間グレー
    static constexpr Color SILVER = { 0xC0C0C0 }; // 明るいグレー

    // UI用定数
    static constexpr Color HP_HIGH = { 0x00FF00 }; // HP高い
    static constexpr Color HP_MID = { 0xFFFF00 }; // HP中
    static constexpr Color HP_LOW = { 0xFF0000 }; // HP低い

    // エフェクト用定数
    static constexpr Color DAMAGE = { 0xFF4444 }; // ダメージ（柔らかい赤）
};