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
    Vector2  operator- (void)                  const;
    Vector2& operator+=(const Vector2& other);
    Vector2& operator-=(const Vector2& other);
    Vector2& operator*=(float scalar);
    Vector2& operator/=(float scalar);
    bool     operator==(const Vector2& other)  const;
    bool     operator!=(const Vector2& other)  const;

    // --- メンバメソッド（自分への操作）---

    // 長さ
    float Length(void)        const;
    float LengthSquared(void) const;

    // 正規化（自分自身を変える）
    void    Normalize(void);
    // 正規化したベクトルを返す（自分は変わらない）
    Vector2 Normalized(void)  const;

    // 判定
    bool IsZero(void)                          const;
    bool IsNearlyZero(float tolerance = 1e-6f) const;
    bool IsNormalized(float tolerance = 1e-4f) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // --- 静的メソッド（2つ以上への操作）---

    // 内積
    static float   Dot(const Vector2& a, const Vector2& b);
    // 距離
    static float   Distance(const Vector2& a, const Vector2& b);
    static float   DistanceSquared(const Vector2& a, const Vector2& b);
    // 線形補間
    static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);

    // 定数（cpp に定義）
    static const Vector2 ZERO;
    static const Vector2 ONE;
    static const Vector2 UP;
    static const Vector2 RIGHT;
};

// スカラーを左に書けるようにする（2.0f * v）
Vector2 operator*(float scalar, const Vector2& v);