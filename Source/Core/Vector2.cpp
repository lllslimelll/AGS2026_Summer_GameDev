#include <cmath>
#include <algorithm>
#include <sstream>
#include "Vector2.h"

// ---------------------------------------------------------------
// 定数の定義
// ---------------------------------------------------------------
const Vector2 Vector2::ZERO = { 0.0f, 0.0f };
const Vector2 Vector2::ONE = { 1.0f, 1.0f };
const Vector2 Vector2::UP = { 0.0f, 1.0f };
const Vector2 Vector2::RIGHT = { 1.0f, 0.0f };

// ---------------------------------------------------------------
// コンストラクタ
// ---------------------------------------------------------------
Vector2::Vector2(float x, float y)
    : x(x), y(y)
{
}

// ---------------------------------------------------------------
// 演算子オーバーロード
// ---------------------------------------------------------------
Vector2 Vector2::operator+(const Vector2& other) const
{
    return { x + other.x, y + other.y };
}

Vector2 Vector2::operator-(const Vector2& other) const
{
    return { x - other.x, y - other.y };
}

Vector2 Vector2::operator*(float scalar) const
{
    return { x * scalar, y * scalar };
}

Vector2 Vector2::operator/(float scalar) const
{
    return { x / scalar, y / scalar };
}

Vector2 Vector2::operator-(void) const
{
    return { -x, -y };
}

Vector2& Vector2::operator+=(const Vector2& other)
{
    x += other.x; y += other.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other)
{
    x -= other.x; y -= other.y;
    return *this;
}

Vector2& Vector2::operator*=(float scalar)
{
    x *= scalar; y *= scalar;
    return *this;
}

Vector2& Vector2::operator/=(float scalar)
{
    x /= scalar; y /= scalar;
    return *this;
}

bool Vector2::operator==(const Vector2& other) const
{
    return x == other.x && y == other.y;
}

bool Vector2::operator!=(const Vector2& other) const
{
    return !(*this == other);
}

// ---------------------------------------------------------------
// 長さ
// ---------------------------------------------------------------
float Vector2::Length(void) const
{
    return std::sqrt(LengthSquared());
}

float Vector2::LengthSquared(void) const
{
    return x * x + y * y;
}

// ---------------------------------------------------------------
// 正規化
// ---------------------------------------------------------------
void Vector2::Normalize(void)
{
    float len = Length();
    if (len > 0.0f)
    {
        x /= len; y /= len;
    }
}

Vector2 Vector2::Normalized(void) const
{
    float len = Length();
    if (len > 0.0f)
    {
        return { x / len, y / len };
    }
    return ZERO;
}

// ---------------------------------------------------------------
// 判定
// ---------------------------------------------------------------
bool Vector2::IsZero(void) const
{
    return x == 0.0f && y == 0.0f;
}

bool Vector2::IsNearlyZero(float tolerance) const
{
    return std::abs(x) <= tolerance
        && std::abs(y) <= tolerance;
}

bool Vector2::IsNormalized(float tolerance) const
{
    return std::abs(LengthSquared() - 1.0f) <= tolerance;
}

// ---------------------------------------------------------------
// デバッグ用文字列化
// ---------------------------------------------------------------
std::string Vector2::ToString(void) const
{
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ")";
    return oss.str();
}

// ---------------------------------------------------------------
// 静的メソッド
// ---------------------------------------------------------------
float Vector2::Dot(const Vector2& a, const Vector2& b)
{
    return a.x * b.x + a.y * b.y;
}

float Vector2::Distance(const Vector2& a, const Vector2& b)
{
    return (a - b).Length();
}

float Vector2::DistanceSquared(const Vector2& a, const Vector2& b)
{
    return (a - b).LengthSquared();
}

Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return
    {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
    };
}

// ---------------------------------------------------------------
// スカラーを左に書けるようにする
// ---------------------------------------------------------------
Vector2 operator*(float scalar, const Vector2& v)
{
    return v * scalar;
}