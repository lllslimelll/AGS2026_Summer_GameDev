#include <cmath>
#include <algorithm>
#include <sstream>
#include "Vector2.h"

Vector2::Vector2(float x, float y)
    : x(x), y(y)
{
}

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

float Vector2::Length(void) const
{
    return std::sqrt(LengthSquared());
}

float Vector2::LengthSquared(void) const
{
    return x * x + y * y;
}

void Vector2::Normalize(void)
{
    float len = Length();
    if (len > 0.0f)
    {
        x /= len; y /= len;
    }
}

Vector2 Vector2::GetNormalized(void) const
{
    float len = Length();
    if (len > 0.0f)
    {
        return { x / len, y / len };
    }
    return ZERO;
}

float Vector2::Dot(const Vector2& other) const
{
    return x * other.x + y * other.y;
}

float Vector2::Distance(const Vector2& other) const
{
    return (*this - other).Length();
}

float Vector2::DistanceSquared(const Vector2& other) const
{
    return (*this - other).LengthSquared();
}

Vector2 Vector2::Lerp(const Vector2& other, float t) const
{
    t = std::clamp(t, 0.0f, 1.0f);
    return
    {
        x + (other.x - x) * t,
        y + (other.y - y) * t,
    };
}

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

std::string Vector2::ToString(void) const
{
    std::ostringstream oss;
    oss << "(" << x << ", " << y << ")";
    return oss.str();
}

float Vector2::Distance(const Vector2& a, const Vector2& b)
{
    return a.Distance(b);
}

float Vector2::DistanceSquared(const Vector2& a, const Vector2& b)
{
    return a.DistanceSquared(b);
}

Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, float t)
{
    return a.Lerp(b, t);
}

Vector2 operator*(float scalar, const Vector2& v)
{
    return v * scalar;
}