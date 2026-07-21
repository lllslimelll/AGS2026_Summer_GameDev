// Core/Math.cpp
#include <cmath>
#include <algorithm>
#include "Math.h"

// ---------------------------------------------------------------
// Šî–{‰‰Z
// ---------------------------------------------------------------
float Math::Clamp(float v, float min, float max)
{
    return std::clamp(v, min, max);
}

int Math::Clamp(int v, int min, int max)
{
    return std::clamp(v, min, max);
}

float Math::Abs(float v)
{
    return std::abs(v);
}

int Math::Abs(int v)
{
    return std::abs(v);
}

float Math::Max(float a, float b)
{
    return std::max(a, b);
}

float Math::Min(float a, float b)
{
    return std::min(a, b);
}

int Math::Max(int a, int b)
{
    return std::max(a, b);
}

int Math::Min(int a, int b)
{
    return std::min(a, b);
}

int Math::Round(float v)
{
    return static_cast<int>(std::roundf(v));
}

float Math::Pow(float base, float exp)
{
    return std::powf(base, exp);
}

float Math::Sqrt(float v)
{
    return std::sqrtf(v);
}

// ---------------------------------------------------------------
// •âŠÔ
// ---------------------------------------------------------------
float Math::Lerp(float start, float end, float t)
{
    t = Clamp(t, 0.0f, 1.0f);
    return start + t * (end - start);
}

int Math::Lerp(int start, int end, float t)
{
    t = Clamp(t, 0.0f, 1.0f);
    return start + Round(t * static_cast<float>(end - start));
}

double Math::Lerp(double start, double end, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return start + t * (end - start);
}

double Math::LerpDeg(double start, double end, double t)
{
    double diff = end - start;
    if (diff < -180.0)
    {
        end += 360.0;
        double ret = Lerp(start, end, t);
        if (ret >= 360.0) ret -= 360.0;
        return ret;
    }
    else if (diff > 180.0)
    {
        end -= 360.0;
        double ret = Lerp(start, end, t);
        if (ret < 0.0) ret += 360.0;
        return ret;
    }
    return Lerp(start, end, t);
}

float Math::Bezier(float p1, float p2, float p3, float t)
{
    float a = Lerp(p1, p2, t);
    float b = Lerp(p2, p3, t);
    return Lerp(a, b, t);
}

// ---------------------------------------------------------------
// Šp“x
// ---------------------------------------------------------------
double Math::DegIn360(double deg)
{
    deg = std::fmod(deg, 360.0);
    if (deg < 0.0) deg += 360.0;
    return deg;
}

double Math::RadIn2PI(double rad)
{
    rad = std::fmod(rad, TWO_PI);
    if (rad < 0.0) rad += TWO_PI;
    return rad;
}

int Math::DirNearAroundRad(float from, float to)
{
    float diff = to - from;
    if (diff >= 0.0f)
    {
        return (diff > PI) ? -1 : 1;
    }
    return (diff < -PI) ? 1 : -1;
}

int Math::DirNearAroundDeg(float from, float to)
{
    float diff = to - from;
    if (diff >= 0.0f)
    {
        return (diff > 180.0f) ? -1 : 1;
    }
    return (diff < -180.0f) ? 1 : -1;
}

// ---------------------------------------------------------------
// ”äŠr
// ---------------------------------------------------------------
bool Math::NearlyEqual(float a, float b, float tolerance)
{
    return std::abs(a - b) <= tolerance;
}

bool Math::NearlyZero(float v, float tolerance)
{
    return std::abs(v) <= tolerance;
}