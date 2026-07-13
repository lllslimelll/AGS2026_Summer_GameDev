#pragma once

// パレット
namespace Palette
{
    // 基本色
    static constexpr unsigned int WHITE = 0xFFFFFF;   // 白
    static constexpr unsigned int BLACK = 0x000000;   // 黒
    static constexpr unsigned int RED = 0xFF0000;     // 赤
    static constexpr unsigned int GREEN = 0x00FF00;   // 緑
    static constexpr unsigned int BLUE = 0x0000FF;    // 青
    static constexpr unsigned int YELLOW = 0xFFFF00;  // 黄色
    static constexpr unsigned int CYAN = 0x00FFFF;    // 青緑
    static constexpr unsigned int MAGENTA = 0xFF00FF; // 赤紫
    static constexpr unsigned int ORANGE = 0xFF8000;  // 橙
    static constexpr unsigned int PURPLE = 0x800080;  // 紫
    static constexpr unsigned int PINK = 0xFF69B4;    // 桃色
    static constexpr unsigned int BROWN = 0x8B4513;   // 茶色
    static constexpr unsigned int GRAY = 0x808080;    // 中間グレー
    static constexpr unsigned int SILVER = 0xC0C0C0;  // 明るいグレー

    // UI用
    static constexpr unsigned int HP_HIGH = 0x00FF00; // HP高い
    static constexpr unsigned int HP_MID = 0xFFFF00;  // HP中
    static constexpr unsigned int HP_LOW = 0xFF0000;  // HP低い

    // エフェクト用
    static constexpr unsigned int DAMAGE = 0xFF4444;  // ダメージ（柔らかい赤）
}