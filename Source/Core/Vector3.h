// Core/Vector3.h
#pragma once
#include <string>
#include <DxLib.h>

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // コンストラクタ
    constexpr Vector3() = default;

    constexpr Vector3(float x, float y, float z)
        : x(x), y(y), z(z)
    {
    }

    // 演算子オーバーロード
    Vector3  operator+ (const Vector3& other) const;
    Vector3  operator- (const Vector3& other) const;
    Vector3  operator* (const Vector3& other) const;
    Vector3  operator* (float scalar)         const;
    Vector3  operator/ (float scalar)         const;
    Vector3  operator- (void)                 const;
    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);
    bool     operator==(const Vector3& other) const;
    bool     operator!=(const Vector3& other) const;

    // --- メンバメソッド（自分への操作）---

    // 長さ
    float Length(void)        const;
    float LengthSquared(void) const;

    // 正規化
    void    Normalize(void);
    // 正規化したベクトルを返す
    Vector3 Normalized(void)  const;

    // 各成分をクランプ
    Vector3 Clamp(const Vector3& min, const Vector3& max) const;

    // 反射ベクトル（normal は正規化済みであること）
    Vector3 Reflect(const Vector3& normal) const;

    // 射影（normal 方向への射影）
    Vector3 Project(const Vector3& normal) const;

    // 判定
    bool IsZero(void)                          const;
    bool IsNearlyZero(float tolerance = 1e-6f) const;
    bool IsNormalized(float tolerance = 1e-4f) const;

    // デバッグ用文字列化
    std::string ToString(void) const;

    // --- 静的メソッド（2つ以上への操作）---

    // 内積
    static float   Dot(const Vector3& a, const Vector3& b);
    // 外積
    static Vector3 Cross(const Vector3& a, const Vector3& b);
    // 距離
    static float   Distance(const Vector3& a, const Vector3& b);
    static float   DistanceSquared(const Vector3& a, const Vector3& b);
    // 線形補間（t: 0.0f?1.0f）
    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);

    // DxLib用の変換
    VECTOR    ToVECTOR(void) const { return { x, y, z }; }
    static Vector3 FromVECTOR(const VECTOR& v) { return { v.x, v.y, v.z }; }

    // 定数
    static const Vector3 ZERO;
    static const Vector3 ONE;
    static const Vector3 UP;
    static const Vector3 DOWN;
    static const Vector3 FORWARD;
    static const Vector3 BACK;
    static const Vector3 RIGHT;
    static const Vector3 LEFT;
};

// スカラーを左に書けるようにする
Vector3 operator*(float scalar, const Vector3& v);