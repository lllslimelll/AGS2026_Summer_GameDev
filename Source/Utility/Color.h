#pragma once
#include <string>

// カラー
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

    // 演算子オーバーロード
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // 基本色定数
    static const Color WHITE;
    static const Color BLACK;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color YELLOW;
    static const Color CYAN;    // 青緑
    static const Color MAGENTA; // 赤紫
    static const Color ORANGE;  // 橙
    static const Color PURPLE;  // 紫
    static const Color PINK;    // 桃色
    static const Color BROWN;   // 茶色
    static const Color GRAY;    // 中間グレー
    static const Color SILVER;  // 明るいグレー

    // UI用定数（cpp に定義）
    static const Color HP_HIGH; // HP高い
    static const Color HP_MID;  // HP中
    static const Color HP_LOW;  // HP低い
};