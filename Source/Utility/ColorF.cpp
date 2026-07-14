#include <cmath>
#include <algorithm>
#include <sstream>
#include "ColorF.h"
#include "Color.h"

ColorF::ColorF(float r, float g, float b)
    : r(r), g(g), b(b), a(1.0f)
{
}

ColorF::ColorF(float r, float g, float b, float a)
    : r(r), g(g), b(b), a(a)
{
}

ColorF ColorF::operator+(const ColorF& other) const
{
    return
    {
        std::clamp(r + other.r, 0.0f, 1.0f),
        std::clamp(g + other.g, 0.0f, 1.0f),
        std::clamp(b + other.b, 0.0f, 1.0f),
        std::clamp(a + other.a, 0.0f, 1.0f),
    };
}

ColorF ColorF::operator*(float scalar) const
{
    return
    {
        std::clamp(r * scalar, 0.0f, 1.0f),
        std::clamp(g * scalar, 0.0f, 1.0f),
        std::clamp(b * scalar, 0.0f, 1.0f),
        std::clamp(a * scalar, 0.0f, 1.0f),
    };
}

ColorF& ColorF::operator+=(const ColorF& other)
{
    r = std::clamp(r + other.r, 0.0f, 1.0f);
    g = std::clamp(g + other.g, 0.0f, 1.0f);
    b = std::clamp(b + other.b, 0.0f, 1.0f);
    a = std::clamp(a + other.a, 0.0f, 1.0f);
    return *this;
}

ColorF& ColorF::operator*=(float scalar)
{
    r = std::clamp(r * scalar, 0.0f, 1.0f);
    g = std::clamp(g * scalar, 0.0f, 1.0f);
    b = std::clamp(b * scalar, 0.0f, 1.0f);
    a = std::clamp(a * scalar, 0.0f, 1.0f);
    return *this;
}

bool ColorF::operator==(const ColorF& other) const
{
    return r == other.r && g == other.g
        && b == other.b && a == other.a;
}

bool ColorF::operator!=(const ColorF& other) const
{
    return !(*this == other);
}

ColorF ColorF::Lerp(const ColorF& other, float t) const
{
    t = std::clamp(t, 0.0f, 1.0f);
    return
    {
        r + (other.r - r) * t,
        g + (other.g - g) * t,
        b + (other.b - b) * t,
        a + (other.a - a) * t,
    };
}

ColorF ColorF::Lerp(const ColorF& a, const ColorF& b, float t)
{
    return a.Lerp(b, t);
}

Color ColorF::ToColor(void) const
{
    return Color(
        static_cast<unsigned char>(r * 255.0f),
        static_cast<unsigned char>(g * 255.0f),
        static_cast<unsigned char>(b * 255.0f),
        static_cast<unsigned char>(a * 255.0f));
}

std::string ColorF::ToString(void) const
{
    std::ostringstream oss;
    oss << "(" << r << ", " << g << ", " << b << ", " << a << ")";
    return oss.str();
}