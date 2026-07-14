#pragma once
#include <DxLib.h>
#include <string>

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // コンストラクタ
    Vector3(void) = default;
    Vector3(float x, float y, float z);

    // 演算子オーバーロード
    Vector3  operator+ (const Vector3& other) const;
    Vector3  operator- (const Vector3& other) const;
    Vector3  operator* (float scalar)         const;
    Vector3  operator/ (float scalar)         const;
    Vector3  operator- (void)                 const;  // 単項マイナス
    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);
    bool     operator==(const Vector3& other) const;
    bool     operator!=(const Vector3& other) const;

    // 長さ
    float Length(void)        const;
    float LengthSquared(void) const;

    // 正規化
    void    Normalize(void);
    Vector3 GetNormalized(void) const;

    // 内積 / 外積
    float   Dot(const Vector3& other)   const;
    Vector3 Cross(const Vector3& other) const;

    // 距離
    float Distance(const Vector3& other)        const;
    float DistanceSquared(const Vector3& other) const;

    // 線形補間
    Vector3 Lerp(const Vector3& other, float t) const;

    // 各成分をクランプ
    Vector3 Clamp(const Vector3& min, const Vector3& max) const;

    // 反射ベクトル（normal は正規化済みであること）
    Vector3 Reflect(const Vector3& normal) const;

    // 射影（normal 方向への射影）
    Vector3 Project(const Vector3& normal) const;

    // 判定
    bool IsZero(void)                               const;
    bool IsNearlyZero(float tolerance = 1e-6f)      const;  // 誤差考慮
    bool IsNormalized(float tolerance = 1e-4f)      const;  // 正規化済みか

    // デバッグ用文字列化
    std::string ToString(void) const;

    // DxLib との変換
    VECTOR         ToVECTOR(void)              const;
    static Vector3 FromVECTOR(const VECTOR& v);

    // 静的メソッド
    static float   Distance(const Vector3& a, const Vector3& b);
    static float   DistanceSquared(const Vector3& a, const Vector3& b);
    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);

    // 定数
    static constexpr Vector3 ZERO = { 0.0f, 0.0f, 0.0f };
    static constexpr Vector3 ONE = { 1.0f, 1.0f, 1.0f };
    static constexpr Vector3 UP = { 0.0f, 1.0f, 0.0f };
    static constexpr Vector3 DOWN = { 0.0f, -1.0f, 0.0f };
    static constexpr Vector3 FORWARD = { 0.0f, 0.0f, 1.0f };
    static constexpr Vector3 BACK = { 0.0f, 0.0f, -1.0f };
    static constexpr Vector3 RIGHT = { 1.0f, 0.0f, 0.0f };
    static constexpr Vector3 LEFT = { -1.0f, 0.0f, 0.0f };
};

// スカラーを左に書けるようにする
Vector3 operator*(float scalar, const Vector3& v);