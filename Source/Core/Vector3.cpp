// Core/Vector3.cpp
#include <cmath>
#include <algorithm>
#include <sstream>
#include "Vector3.h"

// ---------------------------------------------------------------
// 定数の定義
// ---------------------------------------------------------------
const Vector3 Vector3::ZERO = Vector3(0.0f,0.0f,0.0f);
const Vector3 Vector3::ONE = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::UP = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::DOWN = Vector3(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::FORWARD = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::BACK = Vector3(0.0f, 0.0f,-1.0f);
const Vector3 Vector3::RIGHT = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::LEFT = Vector3(-1.0f, 0.0f, 0.0f);

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

Vector3 Vector3::operator*(const Vector3& other) const
{
    return { x * other.x, y * other.y, z * other.z };
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

Vector3 Vector3::Normalized(void) const
{
    float len = Length();
    if (len > 0.0f)
    {
        return { x / len, y / len, z / len };
    }
    return ZERO;
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
// 反射ベクトル（v - 2(v・n)n）
// ---------------------------------------------------------------
Vector3 Vector3::Reflect(const Vector3& normal) const
{
    return *this - normal * (2.0f * Dot(*this, normal));
}

// ---------------------------------------------------------------
// 射影（(v・n)n）
// ---------------------------------------------------------------
Vector3 Vector3::Project(const Vector3& normal) const
{
    return normal * Dot(*this, normal);
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
// 静的メソッド
// ---------------------------------------------------------------
float Vector3::Dot(const Vector3& a, const Vector3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 Vector3::Cross(const Vector3& a, const Vector3& b)
{
    return
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

float Vector3::Distance(const Vector3& a, const Vector3& b)
{
    return (a - b).Length();
}

float Vector3::DistanceSquared(const Vector3& a, const Vector3& b)
{
    return (a - b).LengthSquared();
}

Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return
    {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
    };
}

// ---------------------------------------------------------------
// スカラーを左に書けるようにする
// ---------------------------------------------------------------
Vector3 operator*(float scalar, const Vector3& v)
{
    return v * scalar;
}