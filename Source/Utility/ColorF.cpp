#include <cmath>
#include <algorithm>
#include <sstream>
#include "ColorF.h"
#include "Color.h"

// ---------------------------------------------------------------
// íËêîÇÃíËã`
// ---------------------------------------------------------------
const ColorF ColorF::WHITE = { 1.0f,  1.0f,  1.0f,  1.0f };
const ColorF ColorF::BLACK = { 0.0f,  0.0f,  0.0f,  1.0f };
const ColorF ColorF::RED = { 1.0f,  0.0f,  0.0f,  1.0f };
const ColorF ColorF::GREEN = { 0.0f,  1.0f,  0.0f,  1.0f };
const ColorF ColorF::BLUE = { 0.0f,  0.0f,  1.0f,  1.0f };
const ColorF ColorF::YELLOW = { 1.0f,  1.0f,  0.0f,  1.0f };
const ColorF ColorF::CYAN = { 0.0f,  1.0f,  1.0f,  1.0f };
const ColorF ColorF::MAGENTA = { 1.0f,  0.0f,  1.0f,  1.0f };
const ColorF ColorF::ORANGE = { 1.0f,  0.5f,  0.0f,  1.0f };
const ColorF ColorF::PURPLE = { 0.5f,  0.0f,  0.5f,  1.0f };
const ColorF ColorF::PINK = { 1.0f,  0.41f, 0.71f, 1.0f };
const ColorF ColorF::BROWN = { 0.55f, 0.27f, 0.07f, 1.0f };
const ColorF ColorF::GRAY = { 0.5f,  0.5f,  0.5f,  1.0f };
const ColorF ColorF::SILVER = { 0.75f, 0.75f, 0.75f, 1.0f };

const ColorF ColorF::HP_HIGH = { 0.0f,  1.0f,  0.0f,  1.0f };
const ColorF ColorF::HP_MID = { 1.0f,  1.0f,  0.0f,  1.0f };
const ColorF ColorF::HP_LOW = { 1.0f,  0.0f,  0.0f,  1.0f };

const ColorF ColorF::DAMAGE = { 1.0f,  0.27f, 0.27f, 1.0f };


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

std::string ColorF::ToString(void) const
{
    std::ostringstream oss;
    oss << "(" << r << ", " << g << ", " << b << ", " << a << ")";
    return oss.str();
}