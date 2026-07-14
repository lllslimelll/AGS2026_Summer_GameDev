#pragma once
#include <string>

struct Vector2
{
    float x = 0.0f;
    float y = 0.0f;

    // コンストラクタ
    Vector2(void) = default;
    Vector2(float x, float y);

    // 演算子オーバーロード
    Vector2  operator+ (const Vector2& other) const;
    Vector2  operator- (const Vector2& other) const;
    Vector2  operator* (float scalar)          const;
    Vector2  operator/ (float scalar)          const;
    Vector2  operator- (void)                  const;  // 単項マイナス
    Vector2& operator+=(const Vector2& other);
    Vector2& operator-=(const Vector2& other);
    Vector2& operator*=(float scalar);
    Vector2& operator/=(float scalar);
    bool     operator==(const Vector2& other)  const;
    bool     operator!=(const Vector2& other)  const;

    // 長さ
    float Length(void)        const;
    float LengthSquared(void) const;

    // 正規化
    void    Normalize(void);
    Vector2 GetNormalized(void) const;

    // 内積
    float Dot(const Vector2& other) const;

    // 距離
    float Distance(const Vector2& other)        const;
    float DistanceSquared(const Vector2& other) const;

    // 線形補間
    Vector2 Lerp(const Vector2& other, float t) const;

    // 判定
    bool IsZero(void)                          const;
    bool IsNearlyZero(float tolerance = 1e-6f) const;
    bool IsNormalized(float tolerance = 1e-4f) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // 静的メソッド
    static float   Distance(const Vector2& a, const Vector2& b);
    static float   DistanceSquared(const Vector2& a, const Vector2& b);
    static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);

    // 定数
    static constexpr Vector2 ZERO = { 0.0f, 0.0f };
    static constexpr Vector2 ONE = { 1.0f, 1.0f };
    static constexpr Vector2 UP = { 0.0f, 1.0f };
    static constexpr Vector2 RIGHT = { 1.0f, 0.0f };
};

// スカラーを左に書けるようにする
Vector2 operator*(float scalar, const Vector2& v);