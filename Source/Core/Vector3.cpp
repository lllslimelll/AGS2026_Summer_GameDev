#include <cmath>
#include <algorithm>
#include <sstream>
#include "Vector3.h"

Vector3::Vector3(float x, float y, float z)
    : x(x), y(y), z(z)
{
}

// ---------------------------------------------------------------
// 演算子オーバーロード
// ---------------------------------------------------------------
Vector3 Vector3::operator+(const Vector3& other) const
{
    return { x + other.x, y + other.y, z + other.z };
}

Vector3 Vector3::operator-(const Vector3& other) const
{
    return { x - other.x, y - other.y, z - other.z };
}

Vector3 Vector3::operator*(float scalar) const
{
    return { x * scalar, y * scalar, z * scalar };
}

Vector3 Vector3::operator/(float scalar) const
{
    return { x / scalar, y / scalar, z / scalar };
}

Vector3 Vector3::operator-(void) const
{
    return { -x, -y, -z };
}

Vector3& Vector3::operator+=(const Vector3& other)
{
    x += other.x; y += other.y; z += other.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other)
{
    x -= other.x; y -= other.y; z -= other.z;
    return *this;
}

Vector3& Vector3::operator*=(float scalar)
{
    x *= scalar; y *= scalar; z *= scalar;
    return *this;
}

Vector3& Vector3::operator/=(float scalar)
{
    x /= scalar; y /= scalar; z /= scalar;
    return *this;
}

bool Vector3::operator==(const Vector3& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

bool Vector3::operator!=(const Vector3& other) const
{
    return !(*this == other);
}

// ---------------------------------------------------------------
// 長さ
// ---------------------------------------------------------------
float Vector3::Length(void) const
{
    return std::sqrt(LengthSquared());
}

float Vector3::LengthSquared(void) const
{
    return x * x + y * y + z * z;
}

// ---------------------------------------------------------------
// 正規化
// ---------------------------------------------------------------
void Vector3::Normalize(void)
{
    float len = Length();
    if (len > 0.0f)
    {
        x /= len; y /= len; z /= len;
    }
}

Vector3 Vector3::GetNormalized(void) const
{
    float len = Length();
    if (len > 0.0f)
    {
        return { x / len, y / len, z / len };
    }
    return ZERO;
}

// ---------------------------------------------------------------
// 内積 / 外積
// ---------------------------------------------------------------
float Vector3::Dot(const Vector3& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::Cross(const Vector3& other) const
{
    return
    {
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x,
    };
}

// ---------------------------------------------------------------
// 距離
// ---------------------------------------------------------------
float Vector3::Distance(const Vector3& other) const
{
    return (*this - other).Length();
}

float Vector3::DistanceSquared(const Vector3& other) const
{
    return (*this - other).LengthSquared();
}

// 静的バージョン
float Vector3::Distance(const Vector3& a, const Vector3& b)
{
    return a.Distance(b);
}

float Vector3::DistanceSquared(const Vector3& a, const Vector3& b)
{
    return a.DistanceSquared(b);
}

// ---------------------------------------------------------------
// 線形補間
// ---------------------------------------------------------------
Vector3 Vector3::Lerp(const Vector3& other, float t) const
{
    t = std::clamp(t, 0.0f, 1.0f);
    return
    {
        x + (other.x - x) * t,
        y + (other.y - y) * t,
        z + (other.z - z) * t,
    };
}

// 静的バージョン
Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t)
{
    return a.Lerp(b, t);
}

// ---------------------------------------------------------------
// クランプ
// ---------------------------------------------------------------
Vector3 Vector3::Clamp(const Vector3& min, const Vector3& max) const
{
    return
    {
        std::clamp(x, min.x, max.x),
        std::clamp(y, min.y, max.y),
        std::clamp(z, min.z, max.z),
    };
}

// ---------------------------------------------------------------
// 反射ベクトル
// v - 2(v・n)n
// ---------------------------------------------------------------
Vector3 Vector3::Reflect(const Vector3& normal) const
{
    return *this - normal * (2.0f * Dot(normal));
}

// ---------------------------------------------------------------
// 射影
// (v・n)n
// ---------------------------------------------------------------
Vector3 Vector3::Project(const Vector3& normal) const
{
    return normal * Dot(normal);
}

// ---------------------------------------------------------------
// 判定
// ---------------------------------------------------------------
bool Vector3::IsZero(void) const
{
    return x == 0.0f && y == 0.0f && z == 0.0f;
}

bool Vector3::IsNearlyZero(float tolerance) const
{
    return std::abs(x) <= tolerance
        && std::abs(y) <= tolerance
        && std::abs(z) <= tolerance;
}

bool Vector3::IsNormalized(float tolerance) const
{
    return std::abs(LengthSquared() - 1.0f) <= tolerance;
}

// ---------------------------------------------------------------
// デバッグ用文字列化
// ---------------------------------------------------------------
std::string Vector3::ToString(void) const
{
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
}

// ---------------------------------------------------------------
// DxLib との変換
// ---------------------------------------------------------------
VECTOR Vector3::ToVECTOR(void) const
{
    return VGet(x, y, z);
}

Vector3 Vector3::FromVECTOR(const VECTOR& v)
{
    return { v.x, v.y, v.z };
}

// ---------------------------------------------------------------
// スカラーを左に書けるようにする
// ---------------------------------------------------------------
Vector3 operator*(float scalar, const Vector3& v)
{
    return v * scalar;
}